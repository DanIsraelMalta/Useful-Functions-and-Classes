/**
* an emulated 'std::async' with 'deferred' option with the added value that 
* it doesn't block until thread is finished when returned future is destructed.
**/
template<typename FUNC> auto async_deferred(FUNC&& xi_function) -> std::future<decltype(xi_function())> {
    auto task   = std::packaged_task<decltype(xi_function())()>(std::forward<FUNC>(xi_function));
    auto future = task.get_future();
    std::thread(std::move(task)).detach();
    return std::move(future);
}
