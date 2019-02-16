#include <lastlog.h>
#include <stdio.h>

void read_lastlog ( struct lastlog *ll )
{
    fread(ll, sizeof(struct lastlog), 1, stdin);
}


int main ( int ac, char **av )
{
    struct lastlog ll = { 0 };
    read_lastlog(&ll);
    fprintf(stdout, "%d %10s %10s\n", ll.ll_time, ll.ll_line, ll.ll_host);
    return 0;
}
