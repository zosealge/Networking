int config_read_host(struct host_info *host,const char *conf_file)
{
    const int CONF_BUFFER_SIZE=36;
    char conf_buffer[CONF_BUFFER_SIZE];
    char value_buffer[CONF_BUFFER_SIZE];
    char gethostname[CONF_BUFFER_SIZE];
    uint16_t gettcp_port;
    uint16_t getudp_port;
    int count=1;
    FILE *ini_file;
    ini_file=fopen(conf_file,"r");
    if(ini_file==NULL)
    {
        printf("no %s found...\n",conf_file);
        return -1;
    }
    while(!feof(ini_file))
    {
        fgets(conf_buffer,sizeof(conf_buffer),ini_file);
        if(count==1)
        {
            strncpy(gethostname,conf_buffer,13); // max server name is 13 chars
        }
        else if(count==2)
        {
            strncpy(value_buffer,conf_buffer,5);
            gettcp_port=atoi(value_buffer);
        }
        else if(count==3)
        {
            strncpy(value_buffer,conf_buffer,5);
            getudp_port=atoi(value_buffer);
        }
        count++;
    }
    fclose(ini_file);
    strncpy(host->hostname,gethostname,CONF_BUFFER_SIZE);
    host->tcp_port=gettcp_port;
    host->udp_port=getudp_port;
    return 0;
}