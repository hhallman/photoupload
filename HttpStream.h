#pragma once


const int CONNECTION_ERROR		= 0x0008000;


class HttpStream
{
	CHttpConnection*	connection;
	CHttpFile*			request;
	bool				ownsConnection;
	bool				postMode;
	CArray<byte>		postBuffer;
public:
	HttpStream(CHttpConnection* connection, CString queryString, bool ownsConnection=true, bool postMode=false)
	{
		this->ownsConnection	= ownsConnection;
		this->connection		= connection;
		this->postMode			= postMode;
		if(postMode)
		{
			// {9CEF8854-9CF7-4f19-BE79-989EF60E8C5C}
			static const GUID magicNetworkTag = 
			{ 0x9cef8854, 0x9cf7, 0x4f19, { 0xbe, 0x79, 0x98, 0x9e, 0xf6, 0xe, 0x8c, 0x5c } };
			Write<GUID>(magicNetworkTag);
		}
		if(!connection)
			throw CONNECTION_ERROR;
		if(!(request = connection->OpenRequest(postMode?_T("POST"):_T("GET"), queryString, null, 1, null, null, INTERNET_FLAG_NO_CACHE_WRITE)))
			throw CONNECTION_ERROR;
	}
	~HttpStream()
	{
		if(request) {
			request->Close();
			delete request;
		}
		if(connection && ownsConnection) {
			connection->Close();
			delete connection;
		}
	}

	void StartReading()
	{
		if(postMode) {
			if(!request->SendRequestEx(postBuffer.GetCount()))
				throw CONNECTION_ERROR;
			request->Write(postBuffer.GetData(), postBuffer.GetCount());
			if(!request->EndRequest())
				throw CONNECTION_ERROR;
		}
		else {
			if(!request || !request->SendRequest())
				throw CONNECTION_ERROR;
		}
		// {9CEF8854-9CF7-4f19-BE79-989EF60E8C5C}
		static const GUID magicNetworkTag = 
		{ 0x9cef8854, 0x9cf7, 0x4f19, { 0xbe, 0x79, 0x98, 0x9e, 0xf6, 0xe, 0x8c, 0x5c } };
		GUID tempguid = Read<GUID>();
		if(memcmp(&magicNetworkTag, &tempguid, sizeof(GUID)) != 0)
			throw CONNECTION_ERROR;
	}

	UINT FillBuffer(void* buffer, UINT count)
	{
		UINT read=0;
		while(read<count) {
			UINT r = request->Read(((char*)buffer)+read, count-read);
			read += r;
			if(r==0)
				throw CONNECTION_ERROR;
		}
		return read;
	}

	template<class T>T Read(T& r)
	{
		if(sizeof(T) != FillBuffer(&r, sizeof(T)))
			throw CONNECTION_ERROR;
		return r;
	}

	template<class T>T Read()
	{
		T r;
		if(sizeof(T) != FillBuffer(&r, sizeof(T)))
			throw CONNECTION_ERROR;
		return r;
	}

	UINT ReadBuffer(void* buffer, UINT count)
	{
		return request->Read(buffer, count);
	}

	CStringA ReadStringA()
	{
		DWORD bytes = Read<DWORD>();
		CStringA s;
		if(bytes != FillBuffer(s.GetBuffer(bytes+2), bytes))
			throw CONNECTION_ERROR;
		s.GetBuffer()[bytes] = '\0';
		s.ReleaseBuffer();
		return s;
	}

	CStringW ReadStringW()
	{
		DWORD bytes = Read<DWORD>();
		CStringW s;
		UINT read = FillBuffer(s.GetBuffer(bytes/2+2), bytes);
		if(bytes != read)
			throw CONNECTION_ERROR;
		s.GetBuffer()[bytes/2] = 0;
		s.ReleaseBuffer();
		return s;
	}

	void Write(const void* buffer, size_t size)
	{
		ASSERT(postMode);
		size_t old = postBuffer.GetCount();
		postBuffer.SetSize(old + size);
		memcpy(postBuffer.GetData() + old, buffer, size);
	}

	template<class T>void Write(T data)
	{
        Write(&data, sizeof(T));
	}

	void WriteStringW(CStringW string)
	{
		Write<UINT>(sizeof(WCHAR)*string.GetLength());
		Write((LPCWSTR)string, sizeof(WCHAR)*string.GetLength());
	}

	void WriteStringW(CStringA string)
	{
		CStringW s(string);
		WriteStringW(s);
	}


};

