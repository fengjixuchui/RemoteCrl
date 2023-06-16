#include "pch.h"
#include "CServeSocket.h"

//CServeSocket server;
CServeSocket* CServeSocket::m_instance = NULL;

CServeSocket::CHelper CServeSocket::m_helper;

CServeSocket* pserver = CServeSocket::getInsance();


