#pragma once

#include <functional>

#include "RecordLib/FieldTypes.h"
#include "RecordLib/RecordCopier.h"

namespace SRC {
class TestGenericEngine : public SRC::GenericEngineBase
{
	struct Message
	{
		Message(SRC::GenericEngineBase::MessageType mt, const U16unit* str)
			: m_mt(mt)
			, m_strMessage(str){};

		MessageType m_mt;
		SRC::String m_strMessage;
	};

	mutable std::vector<Message> m_messages;
	const std::function<const U16unit*(int, const U16unit*)> m_initVarInject;
	WString m_tempFilePath{};

public:
	TestGenericEngine();
	explicit TestGenericEngine(std::function<const U16unit*(int, const U16unit*)> initVarInject);

	bool HasMessage(const SRC::String& str, MessageType mt = MessageType::MT_FieldConversionError);
	bool HasMessageStarting(const U16unit* start, MessageType mt = MessageType::MT_FieldConversionError);
	virtual bool Ping() const override;
	virtual void QueueThread(ThreadProc pProc, void* pData) const override;
	virtual long OutputMessage(MessageType mt, const U16unit* pMessage) const override;
	virtual const U16unit* GetInitVar2(int nToolId, const U16unit* pVar) const override;
};

class Matcher
{
	SRC::String m_msg;

public:
	Matcher(const SRC::String& msg);

	std::string toString() const;
	bool match(const SRC::Error& err) const;
};

void SetRecordInfo(
	const std::vector<SRC::E_FieldType>& fieldTypes,
	SRC::RecordInfo& recInfoSrc,
	SRC::RecordInfo& recInfoDst,
	SRC::RecordCopier& recCopier);

}  // namespace SRC
