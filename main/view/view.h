#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED


#include "model/model.h"
#include "view/view_types.h"
#include "gel/pagemanager/page_manager.h"


void view_init(model_t *pmodel,
               void (*flush_cb)(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p),
               void (*read_cb)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data));

int view_get_next_msg(model_t *model, view_message_t *msg, view_event_t *eventcopy);
int view_process_msg(view_page_message_t vmsg, model_t *model);

void view_close_all(void *data);
void view_destroy_all(void *data, void *extra);

pman_view_t view_change_page_extra(model_t *pmodel, const pman_page_t *page, void *extra);
pman_view_t view_change_page(model_t *model, const pman_page_t *page);
pman_view_t view_swap_page_extra(model_t *pmodel, const pman_page_t *page, void *extra);
pman_view_t view_swap_page(model_t *pmodel, const pman_page_t *page);
pman_view_t view_reset_to_page(model_t *model, int id);
pman_view_t view_back(model_t *pmodel);

lv_timer_t *view_register_periodic_timer(size_t period, int code);
void        view_register_object_default_callback_with_number(lv_obj_t *obj, int id, int number);
void        view_register_object_default_callback(lv_obj_t *obj, int id);
void        view_event(view_event_t event);


extern const pman_page_t page_main, page_test_output, page_test_input, page_password, page_test_analogs, page_temp_parameter;


#endif