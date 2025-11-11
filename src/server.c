#define _SERVER_HTTP_IMPLEMENTATION
#include <stdio.h>
#include "macros.h"
#include "string.h"
#include "http_server.h"
#include "dates.h"
#include "hash_map.h"
#include "dyn_arr.h"
#include "set.h"
#include "lists.h"

response_t home(request_t) {
    return (response_t) { 200, HTTP_CONTENT_HTTP, "home: "};
}

response_t push(request_t) {
    return (response_t) { 200, HTTP_CONTENT_HTTP, "push_message"};
}

response_t pop(request_t) {
    return (response_t) { 200, HTTP_CONTENT_HTTP, "pop_message"};
}

#define __DATES_H_TESTS



int main(void) {

    srand(time(NULL));

    inf("hash('%s')=%d","Once",hash_map_hash("Once"));
    inf("hash('%s')=%d","Tale",hash_map_hash("Tale"));


    inf("hash('%s')=%d","file",hash_map_hash("file"));
    inf("hash('%s')=%d","directly",hash_map_hash("directly"));

    inf("hash('%s')=%d","No",hash_map_hash("No"));
    inf("hash('%s')=%d","ON",hash_map_hash("ON"));


    // route_t items[] = {
    //     (route_t) {.route="/",     .callback=home },
    //     (route_t) {.route="/push", .callback=push },
    //     (route_t) {.route="/pop",  .callback=pop  },
    // };

    // route_list_t routes = {items,ARRAY_LEN(items),10};
    // start_server(3303,routes);

    reset_console();

    // int src[] = {4,1,3,2,9,0,7,5};
    // int trg[] = {0,0,0,0,0,0,0,0};
    // size_t n = ARRAY_LEN(src);
    // for(size_t i=0;i<n;++i) {
    //     inf("[before] : %d", src[i]);
    // }
    // sort_asc(trg,src,n);
    // inf("-----------------------------------");
    // for(size_t i=0;i<n;++i) {
    //     inf("[after] : %d", trg[i]);
    // }
    // sort_dsc(trg,src,n);
    // inf("-----------------------------------");
    // for(size_t i=0;i<n;++i) {
    //     inf("[after] : %d", trg[i]);
    // }
    // sort_rnd(trg,src,n);
    // inf("-----------------------------------");
    // for(size_t i=0;i<n;++i) {
    //     inf("[after] : %d", trg[i]);
    // }
    
    // test_date_h_get_week_day();
    // test_date_h_dates_1();
    // test_date_h_dates_2();

    test_hash_map_hash();

    assertf(hash_map_hash("abc") != hash_map_hash("abC"), "abc != abC");
    assertf(hash_map_hash("aBc") != hash_map_hash("abc"), "aBc != abc");
    assertf(hash_map_hash("Abc") != hash_map_hash("abc"), "Abc != abc");
    assertf(hash_map_hash("ABc") != hash_map_hash("abc"), "ABc != abc");
    assertf(hash_map_hash("bca") != hash_map_hash("abc"), "bca != abc");
    assertf(hash_map_hash("bac") != hash_map_hash("cba"), "bac != cba");
    assertf(hash_map_hash("cba") != hash_map_hash("abc"), "cba != abc");
    assertf(hash_map_hash("cba") == hash_map_hash("cba"), "cba == cba");

    // inf("testing dyn arr");
    // test_dyn_arr();
    
    // inf("testing set");
    // test_set();
    
    inf("testing shakespeare ... this might take a while");
    test_hash_map_hash_shakespeare();
    // inf("testing test_hash_map_hash_black_peter ... this might take a while");
    // test_hash_map_hash_black_peter();

    inf("DONE testing");
    
    return 0;
}
