#pragma once
#include "util/callable.hh"
#include <memory>

namespace MetaModule
{

class AsyncThread {
public:
	AsyncThread(Callback &&action);
	AsyncThread();

	void start(unsigned module_id);
	void start(unsigned module_id, Callback &&action);

	void stop();
	void run_once(unsigned module_id);

	~AsyncThread();

private:
	Callback action{};
	struct Internal;
	std::unique_ptr<Internal> internal;
};

} // namespace MetaModule
