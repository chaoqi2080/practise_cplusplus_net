//
// Created by chaoqi on 2021/11/20.
//

#ifndef PRACTISE_CPLUSPLUS_NET_MESSAGE_HEADER_HPP
#define PRACTISE_CPLUSPLUS_NET_MESSAGE_HEADER_HPP
enum CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,

    CMD_ERROR,
};

struct DataHeader
{
    uint16_t cmd;
    uint16_t data_length;
};

struct Login : public DataHeader
{
    Login()
    {
        cmd = CMD_LOGIN;
        data_length = sizeof(Login);
    }
    char user_name[64];
    char user_pwd[64];
    char data[950];
};

struct LoginResult : public DataHeader
{
    LoginResult()
    {
        cmd = CMD_LOGIN_RESULT;
        data_length = sizeof(LoginResult);
    }
    uint16_t code;
    char data[1024];
};

struct Logout : public DataHeader
{
    Logout()
    {
        cmd = CMD_LOGOUT;
        data_length = sizeof(Logout);
    }
    char user_name[64];
};

struct LogoutResult : public DataHeader
{
    LogoutResult()
    {
        cmd = CMD_LOGOUT_RESULT;
        data_length = sizeof(LogoutResult);
    }
    uint16_t code;
};

struct NewUserJoin : public DataHeader
{
    NewUserJoin()
    {
        cmd = CMD_NEW_USER_JOIN;
        data_length = sizeof(NewUserJoin);
    }
    uint32_t user_id;
};

#endif //PRACTISE_CPLUSPLUS_NET_MESSAGE_HEADER_HPP
