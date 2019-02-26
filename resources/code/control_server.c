#include "control_server.h"

int init_server_frame(int frame_size) {
    u16_data_off = (uint16_t) ETH_HLEN;
    pu8a_frame   = (uint8_t *) calloc(1, frame_size);

    if (pu8a_frame == NULL) {
        perror("Could not get memory for the transmit frame\n");
        return ERROR_CREATE_FRAME;
    }

    pu8a_data    = pu8a_frame + u16_data_off;

    return NO_ERROR;
}

void set_server_frame_header(uint8_t* src_addr, uint8_t* dest_addr) {
    (void)memcpy(pu8a_frame, dest_addr, ETH_ALEN);
    (void)memcpy(pu8a_frame + ETH_ALEN, src_addr, ETH_ALEN);
}

void free_server_frame() {
    free(pu8a_frame);
    pu8a_frame = NULL;
    pu8a_data  = NULL;
}

void init_server_data(int data_size) {
    (void)memset(&pu8a_frame[u16_data_off], '\0', data_size);
}

void set_server_data(int data_size, int index, long timestamp) {
    (void)snprintf((char *)&pu8a_frame[u16_data_off],
        data_size,"raw_packet_test %d %ld", index, timestamp);
}

void init_server_socket() {
    s32_sock = -1;
}

int set_server_socket() {
    s32_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (s32_sock == -1) {
        perror("Could not create the socket");

        return ERROR_CREATE_SOCKET;
    }

    return NO_ERROR;
}

int bind_server_socket() {
    int resultFunction = bind(s32_sock,
                   (struct sockaddr *)&s_src_addr,
                   sizeof(s_src_addr));

    if (resultFunction == -1) {
        perror("Could not bind to the socket");

        return ERROR_BIND_SOCKET;
    }

    return NO_ERROR;
}

void close_server_socket() {
    if (s32_sock > 0) {
        close(s32_sock);
    }
}

void init_server_sockaddr_ll() {
    (void)memset(&s_src_addr, 0, sizeof(s_src_addr));
}

void set_server_sockaddr_ll(int32_t nic_index) {
    s_src_addr.sll_family = AF_PACKET;
    /*we don't use a protocol above ethernet layer, just use anything here*/
    s_src_addr.sll_protocol = htons(ETH_P_ALL);
    s_src_addr.sll_ifindex = nic_index;
    s_src_addr.sll_hatype = ARPHRD_ETHER;
    s_src_addr.sll_pkttype = PACKET_OTHERHOST; //PACKET_OUTGOING
    s_src_addr.sll_halen = ETH_ALEN;

    socklen_t u32_src_addr_len = sizeof(s_src_addr);
}

int receive_data(int frame_size) {
    int resultFunction = recvfrom(s32_sock,
                           pu8a_frame,
                           frame_size,
                           0,
                           (struct sockaddr *)&s_src_addr,
                           &u32_src_addr_len);

    if (resultFunction == -1) {
        perror("Socket receive failed");

        return FAIL_RECEIVE_DATA;
    }

    else if (resultFunction < 0) {
        perror("Socket receive, error ");

        return ERROR_RECEIVE_DATA;
    }

    return NO_ERROR;
}

int check_data_from_target(uint8_t* target_address) {
    char* message = strtok(&pu8a_frame[u16_data_off], " ");

    for (int index = 0; index < sizeof(s_src_addr.sll_addr) - 2; index++) {
        if (s_src_addr.sll_addr[index] != target_address[index]) {
                printf("Received by unexpected target : ignore this message\n");

                return NOT_FROM_TARGET;
            }
    }

    return FROM_TARGET;
}

void print_target_mac_addr() {
    printf("  Received data from server : ");

    for (int index = 0; index < sizeof(s_src_addr.sll_addr) - 2; index++) {
        printf("%02x:", s_src_addr.sll_addr[index]);
    }

    printf("\n");
}

void parse_data() {
    char* messageToken = NULL;

    for (int index = 0; index < TOKEN_NUM; index++) {
        sArr[index] = NULL;
    }
    
    messageToken = strtok(&pu8a_frame[u16_data_off], " ");

    for (int index = 0; index < TOKEN_NUM; index++) {
        sArr[index] = messageToken;
        messageToken = strtok(NULL, " ");
    }
}

char* get_packet_string() {
    return sArr[0];
}

char* get_packet_index() {
    return sArr[1];
}

char* get_packet_timestamp() {
    return sArr[2];
}