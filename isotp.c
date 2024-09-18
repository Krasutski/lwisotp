#include "isotp.h"

#if CONFIG_ISOTP_ENABLE_ASSERT == 1U
#    include "assert.h"
#endif /* CONFIG_ISOTP_ENABLE_ASSERT */

///////////////////////////////////////////////////////
///                 STATIC FUNCTIONS                ///
///////////////////////////////////////////////////////

/* st_min to microsecond */
static uint8_t _ms_to_st_min(uint8_t ms) {
    uint8_t st_min;

    st_min = ms;
    if (st_min > 0x7F) {
        st_min = 0x7F;
    }

    return st_min;
}

/* st_min to msec  */
static uint8_t _st_min_to_ms(uint8_t st_min) {
    uint8_t ms;

    if (st_min >= 0xF1 && st_min <= 0xF9) {
        ms = 1;
    } else if (st_min <= 0x7F) {
        ms = st_min;
    } else {
        ms = 0;
    }

    return ms;
}

static int _send_flow_control(isotp_link_t const *link, uint8_t flow_status, uint8_t block_size, uint8_t st_min_ms) {

    isotp_can_message_t message;
    int ret;

    /* setup message  */
    message.as.flow_control.type = ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME;
    message.as.flow_control.fs = flow_status;
    message.as.flow_control.bs = block_size;
    message.as.flow_control.st_min = _ms_to_st_min(st_min_ms);

    /* send message */
#if (CONFIG_ISOTP_FRAME_PADDING == 1U)
    (void)memset(message.as.flow_control.reserve, 0, sizeof(message.as.flow_control.reserve));
    ret = isotp_user_send_can(link->send_arbitration_id, message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(link->send_arbitration_id, message.as.data_array.ptr, 3);
#endif

    return ret;
}

static int _send_single_frame(isotp_link_t const *link, uint32_t id) {

    isotp_can_message_t message;
    int ret;

    /* multi frame message length must greater than 7  */
#if CONFIG_ISOTP_ENABLE_ASSERT == 1U
    assert(link->send_size <= 7);
#endif /* CONFIG_ISOTP_ENABLE_ASSERT */

    /* setup message  */
    message.as.single_frame.type = ISOTP_PCI_TYPE_SINGLE;
    message.as.single_frame.sf_dl = (uint8_t)link->send_size;
    (void)memcpy(message.as.single_frame.data, link->send_buffer, link->send_size);

    /* send message */
#if (CONFIG_ISOTP_FRAME_PADDING == 1U)
    (void)memset(message.as.single_frame.data + link->send_size,
                 0,
                 sizeof(message.as.single_frame.data) - link->send_size);
    ret = isotp_user_send_can(id, message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(id, message.as.data_array.ptr, link->send_size + 1);
#endif

    return ret;
}

static int _send_first_frame(isotp_link_t *link, uint32_t id) {

    isotp_can_message_t message;
    int ret;

    /* multi frame message length must greater than 7  */
#if CONFIG_ISOTP_ENABLE_ASSERT == 1U
    assert(link->send_size > 7);
#endif /* CONFIG_ISOTP_ENABLE_ASSERT */

    /* setup message  */
    message.as.first_frame.type = ISOTP_PCI_TYPE_FIRST_FRAME;
    message.as.first_frame.ff_dl_low = (uint8_t)link->send_size;
    message.as.first_frame.ff_dl_high = (uint8_t)(0x0F & (link->send_size >> 8));
    (void)memcpy(message.as.first_frame.data, link->send_buffer, sizeof(message.as.first_frame.data));

    /* send message */
    ret = isotp_user_send_can(id, message.as.data_array.ptr, sizeof(message));
    if (ISOTP_RET_OK == ret) {
        link->send_offset += sizeof(message.as.first_frame.data);
        link->send_sn = 1;
    }

    return ret;
}

static int _send_consecutive_frame(isotp_link_t *link) {

    isotp_can_message_t message;
    uint16_t data_length;
    int ret;

    /* multi frame message length must greater than 7  */
#if CONFIG_ISOTP_ENABLE_ASSERT == 1U
    assert(link->send_size > 7);
#endif /* CONFIG_ISOTP_ENABLE_ASSERT */

    /* setup message  */
    message.as.consecutive_frame.type = ISOTP_PCI_TYPE_CONSECUTIVE_FRAME;
    message.as.consecutive_frame.sn = link->send_sn;
    data_length = link->send_size - link->send_offset;
    if (data_length > sizeof(message.as.consecutive_frame.data)) {
        data_length = sizeof(message.as.consecutive_frame.data);
    }
    (void)memcpy(message.as.consecutive_frame.data, link->send_buffer + link->send_offset, data_length);

    /* send message */
#if (CONFIG_ISOTP_FRAME_PADDING == 1U)
    (void)memset(message.as.consecutive_frame.data + data_length,
                 0,
                 sizeof(message.as.consecutive_frame.data) - data_length);
    ret = isotp_user_send_can(link->send_arbitration_id, message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(link->send_arbitration_id, message.as.data_array.ptr, data_length + 1);
#endif
    if (ISOTP_RET_OK == ret) {
        link->send_offset += data_length;
        if (++(link->send_sn) > 0x0F) {
            link->send_sn = 0;
        }
    }

    return ret;
}

static int _receive_single_frame(isotp_link_t *link, isotp_can_message_t const *message, uint8_t len) {
    /* check data length */
    if ((0 == message->as.single_frame.sf_dl) || (message->as.single_frame.sf_dl > (len - 1))) {
        ISOTP_DEBUG("Single-frame length too small.");
        return ISOTP_RET_LENGTH;
    }

    /* copying data */
    (void)memcpy(link->receive_buffer, message->as.single_frame.data, message->as.single_frame.sf_dl);
    link->receive_size = message->as.single_frame.sf_dl;

    return ISOTP_RET_OK;
}

static int _receive_first_frame(isotp_link_t *link, isotp_can_message_t const *message, uint8_t len) {
    uint16_t payload_length;

    if (8 != len) {
        ISOTP_DEBUG("First frame should be 8 bytes in length.");
        return ISOTP_RET_LENGTH;
    }

    /* check data length */
    payload_length = message->as.first_frame.ff_dl_high;
    payload_length = (payload_length << 8) + message->as.first_frame.ff_dl_low;

    /* should not use multiple frame transmission */
    if (payload_length <= 7) {
        ISOTP_DEBUG("Should not use multiple frame transmission.");
        return ISOTP_RET_LENGTH;
    }

    if (payload_length > link->receive_buf_size) {
        ISOTP_DEBUG("Multi-frame response too large for receiving buffer.");
        return ISOTP_RET_OVERFLOW;
    }

    /* copying data */
    (void)memcpy(link->receive_buffer, message->as.first_frame.data, sizeof(message->as.first_frame.data));
    link->receive_size = payload_length;
    link->receive_offset = sizeof(message->as.first_frame.data);
    link->receive_sn = 1;

    return ISOTP_RET_OK;
}

static int _receive_consecutive_frame(isotp_link_t *link, isotp_can_message_t const *message, uint8_t len) {
    uint16_t remaining_bytes;

    /* check sn */
    if (link->receive_sn != message->as.consecutive_frame.sn) {
        return ISOTP_RET_WRONG_SN;
    }

    /* check data length */
    remaining_bytes = link->receive_size - link->receive_offset;
    if (remaining_bytes > sizeof(message->as.consecutive_frame.data)) {
        remaining_bytes = sizeof(message->as.consecutive_frame.data);
    }
    if (remaining_bytes > len - 1) {
        ISOTP_DEBUG("Consecutive frame too short.");
        return ISOTP_RET_LENGTH;
    }

    /* copying data */
    (void)memcpy(link->receive_buffer + link->receive_offset, message->as.consecutive_frame.data, remaining_bytes);

    link->receive_offset += remaining_bytes;
    if (++(link->receive_sn) > 0x0F) {
        link->receive_sn = 0;
    }

    return ISOTP_RET_OK;
}

/* return logic true if 'a' is after 'b' */
static bool _isotp_time_after(uint32_t now, uint32_t next_ts) {
    return (((int32_t)(next_ts) - (int32_t)(now)) < 0);
}

///////////////////////////////////////////////////////
///                 PUBLIC FUNCTIONS                ///
///////////////////////////////////////////////////////

isotp_result_t isotp_send(isotp_link_t *link, const uint8_t payload[], uint16_t size) {
    if (link == 0x00) {
        ISOTP_DEBUG("Link is null!");
        return ISOTP_RET_ERROR;
    }
    return isotp_send_with_id(link, link->send_arbitration_id, payload, size);
}

isotp_result_t isotp_send_with_id(isotp_link_t *link, uint32_t id, const uint8_t payload[], uint16_t size) {
    isotp_result_t ret;

    if (link == 0x00) {
        ISOTP_DEBUG("Link is null!");
        return ISOTP_RET_ERROR;
    }

    if (size > link->send_buf_size) {
        ISOTP_DEBUG("Message size too large(%d). Increase Buffer size, now is %d", size, link->send_buf_size);
        return ISOTP_RET_OVERFLOW;
    }

    if (ISOTP_SEND_STATUS_IN_PROGRESS == link->send_status) {
        ISOTP_DEBUG("Abort previous message, transmission in progress.");
        return ISOTP_RET_IN_PROGRESS;
    }

    /* copy into local buffer */
    link->send_size = size;
    link->send_offset = 0;
    (void)memcpy(link->send_buffer, payload, size);

    if (link->send_size < 8) {
        /* send single frame */
        ret = _send_single_frame(link, id);
    } else {
        /* send multi-frame */
        ret = _send_first_frame(link, id);

        /* init multi-frame control flags */
        if (ISOTP_RET_OK == ret) {
            link->send_bs_remain = 0;
            link->send_st_min = 0;
            link->send_wtf_count = 0;
            link->send_timer_st = isotp_user_get_ms();
            link->send_timer_bs = isotp_user_get_ms() + CONFIG_ISOTP_DEFAULT_RESPONSE_TIMEOUT;
            link->send_protocol_result = ISOTP_PROTOCOL_RESULT_OK;
            link->send_status = ISOTP_SEND_STATUS_IN_PROGRESS;
        }
    }

    return ret;
}

void isotp_on_can_message(isotp_link_t *link, uint8_t const *data, uint8_t len) {
    isotp_can_message_t message;
    isotp_result_t ret;

    if (len < 2 || len > 8) {
        return;
    }

    memcpy(message.as.data_array.ptr, data, len);
    memset(message.as.data_array.ptr + len, 0, sizeof(message.as.data_array.ptr) - len);

    switch (message.as.common.type) {
        case ISOTP_PCI_TYPE_SINGLE: {
            /* update protocol result */
            if (ISOTP_RECEIVE_STATUS_IN_PROGRESS == link->receive_status) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXPECTED_PDU;
            } else {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_OK;
            }

            /* handle message */
            ret = _receive_single_frame(link, &message, len);

            if (ISOTP_RET_OK == ret) {
                /* change status */
                link->receive_status = ISOTP_RECEIVE_STATUS_FULL;
            }
            break;
        }
        case ISOTP_PCI_TYPE_FIRST_FRAME: {
            /* update protocol result */
            if (ISOTP_RECEIVE_STATUS_IN_PROGRESS == link->receive_status) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXPECTED_PDU;
            } else {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_OK;
            }

            /* handle message */
            ret = _receive_first_frame(link, &message, len);

            /* if overflow happened */
            if (ISOTP_RET_OVERFLOW == ret) {
                /* update protocol result */
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_BUFFER_OVERFLOW;
                /* change status */
                link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
                /* send error message */
                _send_flow_control(link, PCI_FLOW_STATUS_OVERFLOW, 0, 0);
                break;
            }

            /* if receive successful */
            if (ISOTP_RET_OK == ret) {
                /* change status */
                link->receive_status = ISOTP_RECEIVE_STATUS_IN_PROGRESS;
                /* send fc frame */
                link->receive_bs_count = CONFIG_ISOTP_DEFAULT_BLOCK_SIZE;
                _send_flow_control(link, PCI_FLOW_STATUS_CONTINUE, link->receive_bs_count, CONFIG_ISOTP_DEFAULT_ST_MIN);
                /* refresh timer cs */
                link->receive_timer_cr = isotp_user_get_ms() + CONFIG_ISOTP_DEFAULT_RESPONSE_TIMEOUT;
            }

            break;
        }
        case ISOTP_PCI_TYPE_CONSECUTIVE_FRAME: {
            /* check if in receiving status */
            if (ISOTP_RECEIVE_STATUS_IN_PROGRESS != link->receive_status) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXPECTED_PDU;
                break;
            }

            /* handle message */
            ret = _receive_consecutive_frame(link, &message, len);

            /* if wrong sn */
            if (ISOTP_RET_WRONG_SN == ret) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_WRONG_SN;
                link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
                break;
            }

            /* if success */
            if (ISOTP_RET_OK == ret) {
                /* refresh timer cs */
                link->receive_timer_cr = isotp_user_get_ms() + CONFIG_ISOTP_DEFAULT_RESPONSE_TIMEOUT;

                /* receive finished */
                if (link->receive_offset >= link->receive_size) {
                    link->receive_status = ISOTP_RECEIVE_STATUS_FULL;
                } else {
                    /* send fc when bs reaches limit */
                    if (0 == --link->receive_bs_count) {
                        link->receive_bs_count = CONFIG_ISOTP_DEFAULT_BLOCK_SIZE;
                        _send_flow_control(link,
                                           PCI_FLOW_STATUS_CONTINUE,
                                           link->receive_bs_count,
                                           CONFIG_ISOTP_DEFAULT_ST_MIN);
                    }
                }
            }

            break;
        }
        case ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME:
            /* handle fc frame only when sending in progress  */
            if (ISOTP_SEND_STATUS_IN_PROGRESS != link->send_status) {
                break;
            }

            if (len < ISOTP_MIN_FLOW_CONTROL_FRAME_SIZE) {
                ISOTP_DEBUG("Flow control frame too short.");
                break;
            }

            /* refresh bs timer */
            link->send_timer_bs = isotp_user_get_ms() + CONFIG_ISOTP_DEFAULT_RESPONSE_TIMEOUT;

            /* overflow */
            if (PCI_FLOW_STATUS_OVERFLOW == message.as.flow_control.fs) {
                link->send_protocol_result = ISOTP_PROTOCOL_RESULT_BUFFER_OVERFLOW;
                link->send_status = ISOTP_SEND_STATUS_ERROR;
            }

            /* wait */
            else if (PCI_FLOW_STATUS_WAIT == message.as.flow_control.fs) {
                link->send_wtf_count += 1;
                /* wait exceed allowed count */
                if (link->send_wtf_count > CONFIG_ISOTP_MAX_WFT_NUMBER) {
                    link->send_protocol_result = ISOTP_PROTOCOL_RESULT_WFT_OVERRUN;
                    link->send_status = ISOTP_SEND_STATUS_ERROR;
                }
            }

            /* permit send */
            else if (PCI_FLOW_STATUS_CONTINUE == message.as.flow_control.fs) {
                if (0 == message.as.flow_control.bs) {
                    link->send_bs_remain = ISOTP_INVALID_BS;
                } else {
                    link->send_bs_remain = message.as.flow_control.bs;
                }
                link->send_st_min = _st_min_to_ms(message.as.flow_control.st_min);
                link->send_wtf_count = 0;
            }

            break;
        default:
            break;
    };

    return;
}

isotp_result_t isotp_receive(isotp_link_t *link, uint8_t *payload, const uint16_t payload_size, uint16_t *out_size) {
    uint16_t copy_len;

    if (ISOTP_RECEIVE_STATUS_FULL != link->receive_status) {
        return ISOTP_RET_NO_DATA;
    }

    copy_len = link->receive_size;
    if (copy_len > payload_size) {
        copy_len = payload_size;
    }

    memcpy(payload, link->receive_buffer, copy_len);
    *out_size = copy_len;

    link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;

    return ISOTP_RET_OK;
}

void isotp_init_link(isotp_link_t *link,
                     uint32_t send_id,
                     uint8_t *tx_buf,
                     uint16_t tx_buf_size,
                     uint8_t *rx_buf,
                     uint16_t rx_buf_size) {
    memset(link, 0, sizeof(*link));
    link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
    link->send_status = ISOTP_SEND_STATUS_IDLE;
    link->send_arbitration_id = send_id;
    link->send_buffer = tx_buf;
    link->send_buf_size = tx_buf_size;
    link->receive_buffer = rx_buf;
    link->receive_buf_size = rx_buf_size;

    return;
}

void isotp_poll(isotp_link_t *link) {
    int ret;

    /* only polling when operation in progress */
    if (ISOTP_SEND_STATUS_IN_PROGRESS == link->send_status) {

        /* continue send data */

        /* send data if bs_remain is invalid or bs_remain large than zero */
        bool is_need_to_send = (ISOTP_INVALID_BS == link->send_bs_remain) || (link->send_bs_remain > 0);
        if ((is_need_to_send == true) &&
            /* and if st_min is zero or go beyond interval time */
            ((0 == link->send_st_min) || _isotp_time_after(isotp_user_get_ms(), link->send_timer_st))) {
            ret = _send_consecutive_frame(link);
            if (ISOTP_RET_OK == ret) {
                if (ISOTP_INVALID_BS != link->send_bs_remain) {
                    link->send_bs_remain -= 1;
                }
                link->send_timer_bs = isotp_user_get_ms() + CONFIG_ISOTP_DEFAULT_RESPONSE_TIMEOUT;
                link->send_timer_st = isotp_user_get_ms() + link->send_st_min;

                /* check if send finish */
                if (link->send_offset >= link->send_size) {
                    link->send_status = ISOTP_SEND_STATUS_IDLE;
                }
            } else {
                link->send_status = ISOTP_SEND_STATUS_ERROR;
            }
        }

        /* check timeout */
        if (_isotp_time_after(isotp_user_get_ms(), link->send_timer_bs)) {
            link->send_protocol_result = ISOTP_PROTOCOL_RESULT_TIMEOUT_BS;
            link->send_status = ISOTP_SEND_STATUS_ERROR;
        }
    }

    /* only polling when operation in progress */
    if (ISOTP_RECEIVE_STATUS_IN_PROGRESS == link->receive_status) {

        /* check timeout */
        if (_isotp_time_after(isotp_user_get_ms(), link->receive_timer_cr)) {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_TIMEOUT_CR;
            link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
        }
    }

    return;
}
