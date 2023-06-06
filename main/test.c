// Online C compiler to run C program online
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
char req[255] = "+CENG: 1769,0,57,\"0027E019\",-94,-14,-80,-3,3,\"A794\",1,,-83";
char buffer[255];
char message[255];
char* token;
const char* deviceID = "831789eb-3431-4e29-8288-a2721c5d0d7b";
typedef struct{
    char pci[10];
    char cellID[10];
    int cell;
    char rsrp[10];
    char rsrq[10];
    char snr[10];
} info;
info mqtt_message;

void swap_ele(char* data, int ele_1, int ele_2){
    char temp;
    temp = data[ele_1];
    data[ele_1] = data[ele_2];
    data[ele_2] = temp;
}

uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

int main() {
    // Write C code here
    strcpy(buffer, req);
    //bỏ cụm +CENG
    strtok(buffer, ":");
    token = strtok(NULL,",");
    //eafcn_offset
    token = strtok(NULL, ",");
    //pci
    token = strtok(NULL, ",");
    strcpy(mqtt_message.pci, token);
    //cellID
    token = strtok(NULL, "\"");
    strcpy(mqtt_message.cellID, token);
    swap_ele(mqtt_message.cellID, 6,7);
    swap_ele(mqtt_message.cellID, 4,5);  
    mqtt_message.cell = hex2int(mqtt_message.cellID+5);
    //rsrp
    token = strtok(NULL, ",");
    strcpy(mqtt_message.rsrp, token);
    //rsrq
    token = strtok(NULL, ",");
    strcpy(mqtt_message.rsrq, token);
    //rssi
    token = strtok(NULL, ",");
    //snr
    token = strtok(NULL, ",");
    strcpy(mqtt_message.snr,token);
    sprintf(message,"{\"pci\":%s, \"cellId\": %d, \"rsrp\": %s, \"rsrq\": %s, \"snr\": %s}", mqtt_message.pci, mqtt_message.cell, mqtt_message.rsrp, mqtt_message.rsrq, mqtt_message.snr);
    sprintf(buffer, "AT+CMQPUB=0,\"messages/%s/attributets\",1,0,0,%d,\"%s\"\r\n", deviceID, strlen(message), message);
    printf("%s", buffer);
    return 0;
}