
#ifdef CNETMSG_H_INCLUDED
#else
#define CNETMSG_H_INCLUDED

using namespace std;

class CNetMsg {

public:

	CNetMsg() {};

	void PostSub(string subtopic, string data) {};

	void Post(string data) {};

	void Post(string subject, string data) {};

	void Connect(string _appname) {};

	~CNetMsg() {};

};

#endif