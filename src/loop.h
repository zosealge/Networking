time_t wait_nano(long nano)
{
    time_t start=clock();
    time_t end;
    nanosleep((const struct timespec[]){{0, nano}}, NULL);
    end=clock();
    return end-start;
}

time_t wait_sec(long sec)
{
    time_t start=clock();
    time_t end;
    nanosleep((const struct timespec[]){{sec, 0}}, NULL);
    end=clock();
    return end-start;
}