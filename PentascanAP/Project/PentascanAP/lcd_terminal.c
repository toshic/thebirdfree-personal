 #include "rit128x96x4.h"
#include "lcd_terminal.h"

#define FONT_HEIGHT				( 8 )
#define FONT_WIDTH 				( 6 )
#define LCD_HEIGHT				( 128 )
#define LCD_WIDTH 				( 128 )
#define LCD_HEIGHT_VIEW         ( 96 )
#define mainMAX_ROWS_128					( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96						( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64						( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE						( 15 )
#define ulSSI_FREQUENCY						( 3500000UL )


static int screen_offset;
static int cursor_x, cursor_y;
static unsigned char level = mainFULL_SCALE;

void lcd_terminal_init(void)
{
    RIT128x96x4Init(ulSSI_FREQUENCY);
    lcd_terminal_clear();
}

void lcd_terminal_clear(void)
{
    screen_offset = 0;
    cursor_x = 0;
    cursor_y = 0;
    RIT128x96x4Clear();
    RIT128X96X4Scroll(screen_offset);
}

void lcd_terminal_set_level(unsigned char lvl)
{
    level = lvl & 0x0f;
}

void lcd_terminal_char(int ch)
{
     int clear_line = 0;
    int check_scroll = 0;

 /* process cursor increment */    
    if(ch == '\n'){
        if(cursor_x == 0)
            check_scroll = 1;
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
        clear_line = 1;
    }
    else if(ch == '\r'){
        cursor_x = 0;
    }else if(ch == '\t'){
        cursor_x += 4 * FONT_WIDTH;
        if(cursor_x >= ( LCD_WIDTH - FONT_WIDTH )){
            cursor_x = (cursor_x + FONT_WIDTH) % LCD_WIDTH;
            cursor_y += FONT_HEIGHT;
            clear_line = 1;
        }
    }else{
        RIT128x96x4Char(ch, cursor_x, cursor_y % LCD_HEIGHT, level);
        if(cursor_x == 0)
            check_scroll = 1;
        cursor_x += FONT_WIDTH;
    }
        
/* process cursor_x */        
    if(cursor_x >= ( LCD_WIDTH - FONT_WIDTH )){
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
        clear_line = 1;
    }
    
/* process cursor_y */            
    if(cursor_y > ( LCD_HEIGHT - FONT_HEIGHT ))
        cursor_y = 0;

/* clear new line */
    if(clear_line)
        RIT128x96x4StringDraw("                     ",0,cursor_y,level);
        
/* process scroll */   
    if(!check_scroll)
        return;

    if(screen_offset > cursor_y){
        if(LCD_HEIGHT + cursor_y - screen_offset >= LCD_HEIGHT_VIEW){
            screen_offset += FONT_HEIGHT;
            if(screen_offset >= LCD_HEIGHT)
                screen_offset = 0;
            RIT128X96X4Scroll(screen_offset);
        }
    }else if(cursor_y - screen_offset >= LCD_HEIGHT_VIEW ){
        screen_offset += FONT_HEIGHT;
        if(screen_offset >= LCD_HEIGHT)
            screen_offset = 0;
        RIT128X96X4Scroll(screen_offset);
    }
}

void lcd_terminal_str(char *string)
{
    RIT128x96x4StringDraw(string, cursor_x, cursor_y, level);
}
