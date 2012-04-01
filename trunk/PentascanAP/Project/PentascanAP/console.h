

typedef struct{
    FILE *file; 
    int index;
    int length;
    char buffer[1];
}line_buffer;


int mountSd(void);
void print_http(unsigned long size,char *content,void *pv);
void file_http(unsigned long size,char *content,void *pv);
line_buffer *console_buffer_get(int length);
void console_parse(line_buffer *lb, char ch);

