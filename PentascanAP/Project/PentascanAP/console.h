

typedef struct{
    FILE *file; 
    int index;
    int length;
    char buffer[1];
}line_buffer;


void mountSd(void);
line_buffer *console_buffer_get(int length);
void console_parse(line_buffer *lb, char ch);

