




#include "stdafx.h"
#include "JsonMgr.h"
#include <iostream>

CJsonMgr::CJsonMgr() {
}

CJsonMgr::~CJsonMgr() {

}

void CJsonMgr::setProjectName(std::string name)
{
	this->project_name = name;
}
std::string CJsonMgr::getProjectName()
{
	return project_name;
}

void CJsonMgr::setAppName(std::string name)
{
	send_app_name = name;
}
std::string CJsonMgr::getAppName()
{
	return send_app_name;
}

void CJsonMgr::setSendAppVersion(std::string version)
{
	send_app_version = version;
}
std::string CJsonMgr::getSendAppVersion()
{
	return send_app_version;
}

void CJsonMgr::setRecvAppVersion(std::string version)
{
	recv_app_version = version;
}

std::string CJsonMgr::getRecvAppVersion()
{
	return recv_app_version;
}

void CJsonMgr::setRecvAppName(std::string name)
{
	recv_app_name = name;
}
std::string CJsonMgr::getRecvAppName()
{
	return recv_app_name;
}

void CJsonMgr::setCmdID(std::string cmdID)
{
	cmd_id = cmdID;
}
std::string CJsonMgr::getCmdID()
{
	return cmd_id;
}

void CJsonMgr::setCmdDesc(std::string cmdDesc)
{
	cmd_desc = cmdDesc;
}
std::string CJsonMgr::getCmdDesc()
{
	return cmd_desc;
}
std::string CJsonMgr::getUuid()
{
	return uuid;
}
void CJsonMgr::setUuid(std::string uid)
{
	uuid = uid;
}
void CJsonMgr::setSessionID(std::string id)
{
	this->sessionID = id;
}

std::string CJsonMgr::getSessionID()
{
	return this->sessionID;
}

void CJsonMgr::addParam(char* key, char* value)
{
	cmd_params[key] = value;
}
void CJsonMgr::setParam(std::map< std::string, std::string > dictionary)
{

}

bool CJsonMgr::decodeJson(std::string &enc_cmd) {
	Json::Reader reader;
	Json::Value value;

	if (reader.parse(enc_cmd, value)) {
		if (value["array_params"].empty()) {
			std::cout << "array params are null" << std::endl;
			//return NULL;
		}
		std::string project_name = value["project_name"].asString();
		this->project_name = project_name;

		std::string send_app_name = value["send_app_name"].asString();
		this->send_app_name = send_app_name;

		std::string send_app_version = value["send_app_version"].asString();
		this->send_app_version = send_app_version;

		std::string recv_app_name = value["recv_app_name"].asString();
		this->recv_app_name = recv_app_name;

		std::string recv_app_version = value["recv_app_version"].asString();
		this->recv_app_version = recv_app_version;

		std::string cmd_id = value["cmd_id"].asString();
		this->cmd_id = cmd_id;

		std::string cmd_desc = value["cmd_desc"].asString();
		this->cmd_desc = cmd_desc;

		std::string uuid = value["uuid"].asString();
		this->uuid = uuid;

		Json::Value params = value["array_params"];
		for(unsigned int i = 0; i<params.size(); i++)
		{
			std::string param = params[i]["param"].asString();
			size_t index = param.find(":");
			std::string param_key =  param.substr(0,index);
			std::string param_value = param.substr(index+1);
			//std::cout<< param_key << "===="<<param_value<<std::endl;
			this->cmd_params[param_key] = param_value;
		}

		return true;
	}

	return NULL;
}

bool CJsonMgr::encodeJson(std::string &enc_cmd) {
	Json::Value root;
	Json::Value array_params;
	Json::Value item;

	root["project_name"] = project_name.c_str();
	root["send_app_name"] = send_app_name.c_str();
	root["send_app_version"] = send_app_version.c_str();
	root["recv_app_name"] = recv_app_name.c_str();
	root["recv_app_version"] = recv_app_version.c_str();
	root["cmd_id"] = cmd_id.c_str();
	root["cmd_desc"] = cmd_desc.c_str();
	root["uuid"] = uuid.c_str();

	std::map<std::string, std::string>::iterator iter;
	for (iter = cmd_params.begin(); iter != cmd_params.end(); iter++) {
		//char param_value[100] = { 0 };
		//sprintf(param_value, "%s:%s", iter->first.c_str(), iter->second.c_str());
		std::string paramValue = iter->first + ":" + iter->second;
		item["param"] = paramValue;

		array_params.append(item);
	}
	root["array_params"] = array_params;
	enc_cmd = root.toStyledString();

	return true;
}

