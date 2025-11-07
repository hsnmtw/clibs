#define _SERVER_HTTP_IMPLEMENTATION
#include <stdio.h>
#include "macros.h"
#include "http_server.h"
#include "dates.h"
#include "hash_map.h"

response_t home(request_t) {
    return (response_t) { 200, HTTP_CONTENT_HTTP, strcat("home: "," : ")};
}

response_t push(request_t) {
    return (response_t) { 200, HTTP_CONTENT_HTTP, "push_message"};
}

response_t pop(request_t) {
    return (response_t) { 200, HTTP_CONTENT_HTTP, "pop_message"};
}

#define __DATES_H_TESTS

int main(void) {

    // route_t items[] = {
    //     (route_t) {.route="/",     .callback=home },
    //     (route_t) {.route="/push", .callback=push },
    //     (route_t) {.route="/pop",  .callback=pop  },
    // };

    // route_list_t routes = {items,ARRAY_LEN(items),10};
    // start_server(3303,routes);

    reset_console();
    
    test_date_h_get_week_day();
    test_date_h_dates_1();
    test_date_h_dates_2();

    test_hash_map_hash();

    //hash_map_add(NULL,"","");


    
    return 0;
}
