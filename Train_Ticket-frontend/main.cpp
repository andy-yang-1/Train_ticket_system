#include "Engine.h"
#include <winsock.h>
using namespace std ;

my_system train_ticket_system ;

string real_command ;

#ifdef my_debug

extern    ofstream core_file(CORE_FILE,ios::out) ;

extern    bool stop_core = false ;

#endif

string front_end_answer = "" ;

void initialization() ;

int main()
{
    setbuf(stdout,NULL) ;
    int send_len = 0;
    int recv_len = 0;
    int len = 0;
    //定义发送缓冲区和接受缓冲区
    char send_buf[10000] = {0} ;
    char recv_buf[10000] = {0} ;
    //定义服务端套接字，接受请求套接字
    SOCKET s_server;
    SOCKET s_accept;
    //服务端地址客户端地址
    SOCKADDR_IN server_addr;
    SOCKADDR_IN accept_addr;
    initialization();
    //填充服务端信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(10240);
    //创建套接字
    s_server = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        cout << "fail to load!" << endl;
        WSACleanup();
    }
    else {
        cout << "load success!" << endl;
    }
    //设置套接字为监听状态
    if (listen(s_server, SOMAXCONN) < 0) {
        cout << "fail to listen!" << endl;
        WSACleanup();
    }
    else {
        cout << "listen success!" << endl;
    }
    cout << "listening...." << endl;
    //接受连接请求
    len = sizeof(SOCKADDR);
    if (s_accept == SOCKET_ERROR) {
        cout << "connection fail!" << endl;
        WSACleanup();
        return 0;
    }
    cout << "connection built up" << endl;
    //接收数据
//    freopen("1.in","r",stdin) ;
//    freopen("temp_out.txt","w",stdout) ;
//    while (getline(cin,real_command) ){
//        front_end_answer = "" ;
//        train_ticket_system.process_command(real_command) ;
//        cout << front_end_answer ;
//    }
    int counter = 1 ;
    while(true){
        front_end_answer = "" ;
        memset(recv_buf,0, sizeof(recv_buf)) ;
        s_accept = accept(s_server, (SOCKADDR *)&accept_addr, &len);
        recv_len = recv(s_accept, recv_buf, 10000, 0);
        cout << "command " << counter << ": " << recv_buf << endl ;
        real_command = string(recv_buf)  ;
        train_ticket_system.process_command(real_command) ;
        char tcommand[10000] = {0} ;
        strcpy(tcommand,front_end_answer.c_str());
        printf(tcommand) ;
        tcommand[front_end_answer.size()-1] = 0 ;
        send_len = send(s_accept,tcommand,front_end_answer.size()-1,0) ;
        counter++ ;
    }
}

void initialization() {
    //初始化套接字库
    WORD w_req = MAKEWORD(2, 2);//版本号
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0) {
        cout << "fail to initialize lib!" << endl;
    }
    else {
        cout << "initialization success!" << endl;
    }
    //检测版本号
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
        cout << "err type!" << endl;
        WSACleanup();
    }
    else {
        cout << "right type!" << endl;
    }
    //填充服务端地址信息

}