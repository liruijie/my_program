void TianJin_API();
int handle_receive_data(char *rcv_buf);
void Handle_Data_0x2B(char *rcv_buf);
void Handle_Data_0xCA(char *rcv_buf);
int UpdateSignalParameter(int SignalID);
void *Report_Data(void *arg);

