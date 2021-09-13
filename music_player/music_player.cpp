#include "music_player.hpp"

void Usage()
{
    cout << "./MusicServer [command key] [command value] ..." << endl;
    cout << "   " << "-ip : svr listen ip" << endl;
    cout << "   " << "-port : svr listen port" << endl;
    cout << "   " << "-db_ip : msyql server ip address" << endl;
    cout << "   " << "-db_port : mysql port" << endl;
    cout << "   " << "-db_user : mysql user" << endl;
    cout << "   " << "-db_passwd : mysql password" << endl;
    cout << "   " << "-db_db : which database " << endl;
    cout << "   " << "-h : show usage" << endl;
}

int main(int argc, char* argv[])
{
    if(argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        Usage();
        exit(1);
    }

    string svr_ip, db_ip, db_user, db_db, db_passwd;
    uint16_t svr_port, db_port;
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-ip") == 0 && i + 1 < argc)
            svr_ip = argv[i + 1];
        else if(strcmp(argv[i], "-port") == 0 && i + 1 < argc)
            svr_port = atoi(argv[i + 1]);
        else if(strcmp(argv[i], "-db_ip") == 0 && i + 1 < argc)
            db_ip = argv[i + 1];
        else if(strcmp(argv[i], "-db_port") == 0 && i + 1 < argc)
            db_port = atoi(argv[i + 1]); 
        else if(strcmp(argv[i], "-db_user") == 0 && i + 1 < argc)
            db_user = argv[i + 1];
        else if(strcmp(argv[i], "-db_db") == 0 && i + 1 < argc)
            db_db = argv[i + 1];
        else if(strcmp(argv[i], "-db_passwd") == 0 && i + 1 < argc)
            db_passwd = argv[i + 1];
    }
    cout << "please check info: \n";
    cout << "svr_ip" << svr_ip << endl;
    cout << "svr_pot: " << svr_port << endl;
    cout << "db_ip: " << db_ip << endl;
    cout << "db_port: " << db_port << endl;
    cout << "db_user: " << db_user << endl;
    //cout << "db_passwd: " << db_passwd << endl;
    cout << "db_db: " << db_db << endl;
    
    int select = 0;
    cout << "enter check result: currect input 1, error input 0;" << endl;
    cout << "[input select]:";
    cin >> select;

    switch(select)
    {
        case 0:
            cout << "palese retry start server!" << endl;
            break;
        case 1 :
            {
                MusicServer* ms = new MusicServer();
                if(ms == NULL)
                    return -1;
                if(ms->InitMusicServer(db_ip, db_port, db_user, db_passwd, db_db, svr_ip, svr_port) < 0)
                    return -2;
                ms->StartMusicServer();
                delete ms;
                break;
            }
        default:
            printf("input select num error, please input 1(currect) or 0(error)\n");
            break;
    }

    return 0;
}
