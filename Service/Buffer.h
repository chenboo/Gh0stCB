#pragma once
#include <windows.h>

class CBuffer
{
public:
	CBuffer();
	virtual ~CBuffer();

	UINT Read(PBYTE pData, UINT nSize);
	BOOL Write(PBYTE pData, UINT nSize);
	UINT Delete(UINT nSize);

	UINT GetBufferLen();
	PBYTE GetBuffer(UINT nPos = 0);
	void ClearBuffer();

	BOOL Insert(PBYTE pData, UINT nSize);
	void Copy(CBuffer& buffer);	
	int Scan(PBYTE pScan,UINT nPos);

protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);
	UINT DeAllocateBuffer(UINT nRequestedSize);
	UINT GetMemSize();

protected:
	PBYTE	m_pBase;
	PBYTE	m_pPtr;
	UINT	m_nSize;

private:
	CRITICAL_SECTION m_cs;

};
