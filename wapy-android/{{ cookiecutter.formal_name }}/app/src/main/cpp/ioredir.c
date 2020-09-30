static FILE *stdout_redir=NULL;
char stdout_redir_fn[128];
// default is to redirect stdout to logcat with LOG_TAG prefix in INFORMATION level.
static bool stdout_redir_enabled = True;

#if 0
void do_flush_stdout(){
    int has_data = 0;
    int max = sizeof(cstr)-1; // keep room for a null termination

    cstr[0]=0;
    char *cout = &cstr[6];
    int ch;

    (void)fflush(stdout);

    while ( (ch = getc(stdout_redir))  != EOF) {
        if (ch==10){
            // ch = "↲";
            *cout++ = 0xe2;
            *cout++ = 0x86;
            *cout++ = 0xb2;
        }
        *cout++ = (unsigned char)ch;
        has_data++;
        if (has_data==max) {
            LOG_E("Buffer overrun in c-logger redirector");
            break;
        }
    }

    if (has_data) {
        // put a terminal zero.
        *cout = 0;
        const char*label = "cout: ";
        memcpy(cstr,label,strlen(label));
        LOG(LOG_TAG, cstr);
    }

}

void do_stdout_redir(){
    fflush(NULL);

    if (stdout_redir) {
        do_flush_stdout();
        fclose(stdout_redir);
    }

    fclose(stdout);
    //freopen( "/proc/self/fd/0" , "rw", stdin );

    freopen( stdout_redir_fn , "w+", stdout );

    setbuf(stdout, NULL);
    setvbuf (stdout, NULL, _IONBF, BUFSIZ);

    stdout_redir = fopen(stdout_redir_fn,"r");
    setbuf(stdout_redir, NULL);
    setvbuf (stdout_redir, NULL, _IONBF, BUFSIZ);
}
#else

#include <fcntl.h>

int stdio_pfd[2];

static bool stdio_redirected = False;


//    setvbuf(stdout, 0, _IOLBF, 0); // stdout: line-buffered

void do_stdout_redir() {
    if (!stdio_redirected) {
        setvbuf(stdout, 0, _IONBF, 0); // stdout: unbuffered
        setvbuf(stderr, 0, _IONBF, 0); // stderr: unbuffered

        // create the pipe and redirect stdout and stderr
        pipe(stdio_pfd);
        dup2(stdio_pfd[1], 1);
        dup2(stdio_pfd[1], 2);


        if ( fcntl( stdio_pfd[0], F_SETFL, fcntl(stdio_pfd[0], F_GETFL) | O_NONBLOCK) ) {
             LOG_E("stdio redirect failed");
        } else {
            stdio_redirected = True;
            LOG_V("stdio redirected");

        }
    }
}

#define IOBASE IO_MAX - IO_BUFSIZE

void do_flush_stdout(){

    ssize_t has_data = 0;
    int max = sizeof(cstr)-1; // keep room for a null termination

    int ch;

    (void)fflush(stdout);

    /*
    if (!stdio_redirected) {
        LOG(LOG_TAG, " ---------- io redir failure ----------");
        return;
    }
    */

    has_data = read( stdio_pfd[0], &cstr[IOBASE] , IO_BUFSIZE );

    if (has_data > 0) {

        const char * label = "cout: ";
        memcpy(cstr, label, strlen(label));

        char *cout = &cstr[strlen(label)];

        int end_buf = IOBASE+has_data;

        //end_buf cannot be > IO_MAX

        for (int i = IOBASE; i < end_buf ; i++) {
            int ch = cstr[i];
            if (ch == 10){
                // ch = "↲";
                *cout++ = 0xe2;
                *cout++ = 0x86;
                *cout++ = 0xb2;
            } else
                *cout++ = (unsigned char)ch;
        }

        // always put a terminal zero.
        *cout = 0;
        LOG(LOG_TAG, cstr);

        // mark buffer ready for other IO use
        cstr[0] = 0;
    }
}












































#endif
