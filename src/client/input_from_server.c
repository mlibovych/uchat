#include "uchat.h"

t_message *mx_find_message(t_message *messages, int id) {
   t_message *head = messages;
   t_message *node = NULL;

    while (head != NULL) {
        if (head->id == id) {
            node = head;
            break;
        }
        head = head->next;
    }
    return node;
}

static void part1 (int type, json_object *obj, t_client_info *info) {
    if (type == MX_MSG_TYPE)
        mx_input_message(info, obj);
    else if (type == MX_FILE_DOWNLOAD_TYPE) 
        mx_save_file_in_client(info, obj);
    else if (type == MX_AUTH_TYPE_V || type == MX_AUTH_TYPE_NV) 
        mx_input_authentification(info, obj);
    else if (type == MX_LOAD_MORE_TYPE)
        mx_load_history_client(info, obj);
    else if (type == MX_DELETE_MESSAGE_TYPE)
        mx_delete_message_client(info, obj);
    else if (type == MX_EDIT_MESSAGE_TYPE)
        mx_edit_message_client(info, obj);
    else if (type == MX_LOAD_PROFILE_TYPE)
        mx_load_user_profile(info, obj);
}

static void part2 (int type, json_object *obj, t_client_info *info) {
    if (type == MX_LEAVE_ROOM_TYPE)
        mx_leave_room_client(info, obj);
    else if (type == MX_JOIN_ROOM_TYPE)
        mx_join_room_client(info, obj);
    else if (type == MX_CREATE_ROOM_TYPE)
        mx_join_room_client(info, obj);
    else if (type == MX_EDIT_PROFILE_TYPE)
        mx_edit_profile_client(info, obj);
    else if (type == MX_SEARCH_ALL_TYPE)
        mx_search_all_client(info, obj);
    else if (type == MX_DIRECT_MESSAGE_TYPE)
        mx_direct_message_client(info, obj);
}

static int run_function_type_in_client(t_client_info *info, json_object *obj) {
    int type = json_object_get_int(json_object_object_get(obj, "type"));
    // if (type != MX_FILE_DOWNLOAD_TYPE) mx_print_json_object(obj, "mx_process_input_from_server");

    part1(type, obj, info);
    part2(type, obj, info);
    return 0;
}

void *mx_process_input_from_server(void *taken_info) {
    t_client_info *info = (t_client_info *)taken_info;
    int rc;
    char buffer[2048];
    json_tokener *tok = json_tokener_new();
    enum json_tokener_error jerr;
    json_object *new_json;

    while (1) { // read all input from server
        rc = tls_read(info->tls_client, buffer, sizeof(buffer));    // get json
        if (rc == -1) {
            if (mx_reconnect_client(info))
                break;
        }
        else if (rc != 0) {
            new_json = json_tokener_parse_ex(tok, buffer, rc);
            jerr = json_tokener_get_error(tok);
            if (jerr == json_tokener_success)
                run_function_type_in_client(info, new_json);
            json_object_put(new_json);
        }
    }
    return NULL;
}
