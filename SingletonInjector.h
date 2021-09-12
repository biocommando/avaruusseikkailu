#pragma once

template <typename T>
T *get_or_set_singleton(T *ptr)
{
    static T *inst = nullptr;
    if (ptr)
        inst = ptr;
    assert (inst != nullptr);
    return inst;
}

template <typename T>
T *get_singleton()
{
    return get_or_set_singleton<T>(nullptr);
}

template <typename T>
void set_singleton(T *s)
{
    get_or_set_singleton<T>(s);
}