#include "app.h"
void p_a()
{
    char buf[100];
    FILE *fp = fopen("/proc/my_info", "r");
    while (fgets(buf, 100, fp) != NULL)
        printf("%s", buf);
    fclose(fp);
}

void p_v()
{
    char buf[100];
    FILE *fp = fopen("/proc/my_info", "r");
    int start = 0;
    printf("\n");
    while (fgets(buf, 100, fp) != NULL)
    {
        if (buf[0] == '=')
        {
            ++start;
            continue;
        }
        if (start > 1)
            break;
        if (start == 1)
            printf("%s", buf);
    }
    fclose(fp);
}
void p_c()
{
    char buf[100];
    FILE *fp = fopen("/proc/my_info", "r");
    int start = 0;
    printf("\n");
    while (fgets(buf, 100, fp) != NULL)
    {
        if (buf[0] == '=')
        {
            ++start;
            continue;
        }
        if (start > 2)
            break;
        if (start == 2)
            printf("%s", buf);
    }
    fclose(fp);
}
void p_m()
{
    char buf[100];
    FILE *fp = fopen("/proc/my_info", "r");
    int start = 0;
    printf("\n");
    while (fgets(buf, 100, fp) != NULL)
    {

        if (buf[0] == '=')
        {
            ++start;
            continue;
        }
        if (start > 3)
            break;
        if (start == 3)
            printf("%s", buf);
    }
    fclose(fp);
}
void p_t()
{
    char buf[100];
    FILE *fp = fopen("/proc/my_info", "r");
    int start = 0;
    printf("\n");
    while (fgets(buf, 100, fp) != NULL)
    {

        if (buf[0] == '=')
        {
            ++start;
            continue;
        }
        if (start > 4)
            break;
        if (start == 4)
            printf("%s", buf);
    }
    fclose(fp);
}
int main()
{
    char c;
    while (1)
    {
        printf("Which information do you want?\n");
        printf("Version(v),CPU(c),Memory(m),Time(t),All(a),Exit(e)?\n");
        if(scanf("%1s", &c))
        {
            if (c =='a')
                p_a();
            else if(c =='v')
                p_v();
            else if (c == 'c')
                p_c();
            else if (c == 'm')
                p_m();
            else if (c == 't')
                p_t();
            else if (c == 'e')
                return 0;
        }
        printf("-------------------------------------------------\n");
    }
    return 0;
}
