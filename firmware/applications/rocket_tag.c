#include <sysinit.h>
#include "basic/basic.h"
#include "filesystem/ff.h"
#include "lcd/print.h"
#include "lcd/render.h"
#include "funk/nrf24l01p.h"
#include "basic/random.h"
#include "applications/uuid.h"


#define FPS 20
#define GAME_SPEED (1000 / FPS)
#define NICK_LEN 32
#define NICK_BUFFER_SIZE 5

#define MAX_AGE (FPS / 1)

typedef void(*StateFunc)(void);
StateFunc current_state = NULL;

char player_input = 0;

#define pressed_enter (player_input & BTN_ENTER)
#define pressed_right (player_input & BTN_RIGHT)
#define pressed_left  (player_input & BTN_LEFT)
#define pressed_up    (player_input & BTN_UP)
#define pressed_down  (player_input & BTN_DOWN)

char nick[NICK_LEN] =  "Anonymous";

typedef struct 
{
    unsigned long age;
    char nick[NICK_LEN];
}
RadarNick;

RadarNick radar_buffer[NICK_BUFFER_SIZE];

void debug_crash(char *str)
{
    while(true){ 
        lcdClear();
        lcdPrintln(str);
        lcdDisplay();
    }
}

void init_radar_buffer()
{
    for (int i = 0; i < NICK_BUFFER_SIZE; i++)
    {
        radar_buffer[i].age = 0;
        strcpy(radar_buffer[i].nick, "");
    }
}

void draw_radar()
{
    for (int i = 0; i < NICK_BUFFER_SIZE; i++)
    {
        char buff[NICK_LEN + 16];
        if (radar_buffer[i].age < MAX_AGE && strcmp("", radar_buffer[i].nick) != 0)
            sprintf(buff, "* %s", radar_buffer[i].nick);
        else
            sprintf(buff, "  %s", radar_buffer[i].nick);
        lcdPrintln(buff);
    }
}

void increment_age()
{
    for (int i = 0; i < NICK_BUFFER_SIZE; i++)
        radar_buffer[i].age++;
}

void load_nick(){
    FIL file;
    int readbytes;
    f_open(&file, "NICK.CFG", FA_OPEN_EXISTING|FA_READ);
    f_read(&file, &nick, NICK_LEN - 1, &readbytes);
    nick[readbytes] = 0;
}

void found_rocket(char *new_nick)
{
    int oldest = 0;
    for (int i = 0; i < NICK_BUFFER_SIZE; i++)
    {
        if(strcmp(new_nick, radar_buffer[i].nick) == 0)
        {
            radar_buffer[i].age = 0;
            return;
        }
        if (radar_buffer[i].age > radar_buffer[oldest].age)
            oldest = i;
    }
    radar_buffer[oldest].age = 0;
    strcpy(radar_buffer[oldest].nick, new_nick);
}

void find_rockets()
{
    char buf[NICK_LEN];
    nrf_rcv_pkt_start();
    int max_time = (getRandom() % 50) + 50; // avoid sync rockets
    if (nrf_rcv_pkt_time(max_time, NICK_LEN, buf))
    {
        buf[NICK_LEN] = "\0";
        found_rocket(buf);
    }
    nrf_rcv_pkt_end();
}

void run_game()
{
    lcdPrintln(nick);
    char buff[NICK_LEN + 16];
    sprintf(buff, "id: %s", uuid); 
    lcdPrintln(buff);
    lcdPrintln("((- RADAR -))");
    increment_age();
    find_rockets();
    draw_radar();
    for(int i=0; i < 4; i++)
        nrf_snd_pkt_crc(NICK_LEN, nick);
}

void title_screen()
{
    if (pressed_enter)
    {
        current_state = &run_game;
        return;
    }
    DoString(30, 15, "BADGE");
    DoString(20, 25, "ASSASSIN");
    DoString(10, 50, "press enter");
}

void main_rocket_tag() 
{
    backlightSetBrightness(99);
    init_radar_buffer();
    load_nick();
    nrf_init();
    delayms(100);
    nrf_set_channel(81);
    nrf_set_tx_mac(strlen("MEEOO"), (uint8_t*) "MEEOO");
    nrf_set_rx_mac(0,32,strlen("MEEOO"), (uint8_t*) "MEEOO");
    current_state = &title_screen;
    while(true) // main loop
    { 
        lcdClear();
        player_input = getInputRaw();
        current_state();
        lcdDisplay();
        delayms(GAME_SPEED);
    }
}
