#include <stdlib.h>
#include "gel/collections/queue.h"
#include "gel/pagemanager/page_manager.h"
#include "config/app_config.h"
#include "model/model.h"
#include "view.h"
#include "theme/style.h"
#include "theme/theme.h"
#include "utils/utils.h"
#include "gel/timer/timecheck.h"
#include "esp_log.h"


#define DISPLAY_HORIZONTAL_RESOLUTION LV_HOR_RES_MAX
#define DISPLAY_VERTICAL_RESOLUTION   LV_VER_RES_MAX
#ifdef SIMULATOR
#define BUFFER_SIZE (DISPLAY_HORIZONTAL_RESOLUTION * 200)
#else
#include "lvgl_helpers.h"
#define BUFFER_SIZE DISP_BUF_SIZE
#endif


QUEUE_DECLARATION(event_queue, view_event_t, 20);
QUEUE_DEFINITION(event_queue, view_event_t);


static void periodic_timer_callback(lv_timer_t *timer);
static void free_user_data_callback(lv_event_t *event);
static void event_callback(lv_event_t *event);


static const char        *TAG = "View";
static struct event_queue q;
static page_manager_t     pman;
static lv_indev_t        *touch_indev;


void view_init(model_t *pmodel,
               void (*flush_cb)(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p),
               void (*read_cb)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)) {
    (void)TAG;
    lv_init();

    /*A static or global variable to store the buffers*/
    static lv_disp_draw_buf_t disp_buf;

    /*Static or global buffer(s). The second buffer is optional*/
    static lv_color_t buf_1[BUFFER_SIZE] = {0};

    /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
    lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, BUFFER_SIZE);

    static lv_disp_drv_t disp_drv;                     /*A variable to hold the drivers. Must be static or global.*/
    lv_disp_drv_init(&disp_drv);                       /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf;                     /*Set an initialized buffer*/
    disp_drv.flush_cb = flush_cb;                      /*Set a flush callback to draw to the display*/
    disp_drv.hor_res  = DISPLAY_HORIZONTAL_RESOLUTION; /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res  = DISPLAY_VERTICAL_RESOLUTION;   /*Set the vertical resolution in pixels*/

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
    style_init();
    theme_init(disp);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type                   = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb                = read_cb;
    indev_drv.long_press_time        = 1200UL;
    indev_drv.long_press_repeat_time = 50UL;
    /*Register the driver in LVGL and save the created input device object*/
    touch_indev = lv_indev_drv_register(&indev_drv);

    pman_init(&pman);
    event_queue_init(&q);
}


void view_wait_release(void) {
    lv_indev_wait_release(touch_indev);
}


pman_view_t view_change_page_extra(model_t *pmodel, const pman_page_t *page, void *extra) {
    event_queue_init(&q);     // Butta tutti gli eventi precedenti quando cambi la pagina
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    view_wait_release();
    return pman_change_page_extra(&pman, pmodel, *page, extra);
}


pman_view_t view_change_page(model_t *pmodel, const pman_page_t *page) {
    return view_change_page_extra(pmodel, page, NULL);
}


pman_view_t view_swap_page_extra(model_t *pmodel, const pman_page_t *page, void *extra) {
    event_queue_init(&q);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    view_wait_release();
    return pman_swap_page_extra(&pman, pmodel, *(pman_page_t *)page, extra);
}


pman_view_t view_swap_page(model_t *pmodel, const pman_page_t *page) {
    return view_swap_page_extra(pmodel, page, NULL);
}


pman_view_t view_back(model_t *pmodel) {
    event_queue_init(&q);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    view_wait_release();
    return pman_back(&pman, pmodel);
}


pman_view_t view_rebase_page(model_t *pmodel, const pman_page_t *page) {
    event_queue_init(&q);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    view_wait_release();
    return pman_rebase_page(&pman, pmodel, *(pman_page_t *)page);
}


pman_view_t view_reset_to_page(model_t *pmodel, int id) {
    event_queue_init(&q);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    view_wait_release();
    return pman_reset_to_page(&pman, pmodel, id);
}


int view_get_next_msg(model_t *pmodel, view_message_t *msg, view_event_t *eventcopy) {
    view_event_t event;
    int          found = 0;

    if (!event_queue_is_empty(&q)) {
        event_queue_dequeue(&q, &event);
        found = 1;
    }

    if (found) {
        *msg = pman.current_page.process_event(pmodel, pman.current_page.data, event);
        if (eventcopy) {
            *eventcopy = event;
        }
    }

    return found;
}


int view_process_msg(view_page_message_t vmsg, model_t *pmodel) {
    switch (vmsg.code) {
        case VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE:
            view_change_page(pmodel, vmsg.page);
            break;

        case VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA:
            view_change_page_extra(pmodel, vmsg.page, vmsg.extra);
            break;

        case VIEW_PAGE_MESSAGE_CODE_BACK:
            view_back(pmodel);
            break;

        case VIEW_PAGE_MESSAGE_CODE_REBASE:
            view_rebase_page(pmodel, vmsg.page);
            break;

        case VIEW_PAGE_MESSAGE_CODE_SWAP:
            view_swap_page(pmodel, vmsg.page);
            break;

        case VIEW_PAGE_MESSAGE_CODE_SWAP_EXTRA:
            view_swap_page_extra(pmodel, vmsg.page, vmsg.extra);
            break;

        case VIEW_PAGE_MESSAGE_CODE_RESET_TO:
            view_reset_to_page(pmodel, vmsg.id);
            break;

        case VIEW_PAGE_MESSAGE_CODE_NOTHING:
            break;
    }
    return 0;
}


void view_event(view_event_t event) {
    if (event_queue_enqueue(&q, &event)) {
        ESP_LOGI(TAG, "View event queue was full!");
    }
}


void view_destroy_all(void *data, void *extra) {
    free(data);
    free(extra);
}


void view_close_all(void *data) {
    (void)data;
    lv_obj_clean(lv_scr_act());
}


lv_timer_t *view_register_periodic_timer(size_t period, int code) {
    lv_timer_t *timer = lv_timer_create(periodic_timer_callback, period, (void *)(uintptr_t)code);
    lv_timer_pause(timer);
    return timer;
}


void view_register_object_default_callback(lv_obj_t *obj, int id) {
    view_register_object_default_callback_with_number(obj, id, 0);
}


void view_register_object_default_callback_with_number(lv_obj_t *obj, int id, int number) {
    view_object_data_t *data = malloc(sizeof(view_object_data_t));
    data->id                 = id;
    data->number             = number;
    lv_obj_set_user_data(obj, data);
    lv_obj_remove_event_cb(obj, free_user_data_callback);
    lv_obj_remove_event_cb(obj, event_callback);
    lv_obj_add_event_cb(obj, free_user_data_callback, LV_EVENT_DELETE, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_CANCEL, NULL);
    lv_obj_add_event_cb(obj, event_callback, LV_EVENT_READY, NULL);
}


static void free_user_data_callback(lv_event_t *event) {
    if (lv_event_get_code(event) == LV_EVENT_DELETE) {
        lv_obj_t           *obj  = lv_event_get_current_target(event);
        view_object_data_t *data = lv_obj_get_user_data(obj);
        free(data);
    }
}


static void event_callback(lv_event_t *event) {
    view_object_data_t *data       = lv_obj_get_user_data(lv_event_get_current_target(event));
    view_event_t        pman_event = {.code = VIEW_EVENT_CODE_LVGL, .data = *data, .event = lv_event_get_code(event)};

    lv_obj_t *obj = lv_event_get_current_target(event);
    if (lv_obj_check_type(obj, &lv_btn_class)) {
        pman_event.value = lv_obj_has_state(obj, LV_STATE_CHECKED);
    } else if (lv_obj_check_type(obj, &lv_dropdown_class)) {
        pman_event.value = lv_dropdown_get_selected(obj);
        lv_obj_t *list   = lv_dropdown_get_list(obj);
        if (list != NULL) {
            lv_obj_set_width(list, lv_obj_get_width(obj));
        }
    } else if (lv_obj_check_type(obj, &lv_arc_class)) {
        pman_event.value = lv_arc_get_value(obj);
    } else if (lv_obj_check_type(obj, &lv_switch_class)) {
        pman_event.value = lv_obj_has_state(obj, LV_STATE_CHECKED);
    } else if (lv_obj_check_type(obj, &lv_roller_class)) {
        pman_event.value = lv_roller_get_selected(obj);
    } else if (lv_obj_check_type(obj, &lv_textarea_class)) {
        pman_event.string_value = lv_textarea_get_text(obj);
    } else if (lv_obj_check_type(obj, &lv_slider_class)) {
        pman_event.value = lv_slider_get_value(obj);
    } else if (lv_obj_check_type(obj, &lv_keyboard_class)) {
        lv_obj_t *ta = lv_keyboard_get_textarea(obj);
        if (ta != NULL) {
            pman_event.string_value = lv_textarea_get_text(ta);
        }
    } else if (lv_obj_check_type(obj, &lv_btnmatrix_class) && lv_event_get_code(event) == LV_EVENT_VALUE_CHANGED) {
        pman_event.value = lv_btnmatrix_get_selected_btn(obj);
    } else if (lv_obj_check_type(obj, &lv_msgbox_class)) {
        pman_event.value = lv_msgbox_get_active_btn(obj);
    }

    view_event(pman_event);
}


static void periodic_timer_callback(lv_timer_t *timer) {
    int code = (int)(uintptr_t)timer->user_data;
    view_event((view_event_t){.code = VIEW_EVENT_CODE_TIMER, .timer_code = code});
}