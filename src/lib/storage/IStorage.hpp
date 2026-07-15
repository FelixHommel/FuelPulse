#ifndef FUL_SRC_LIB_STORAGE_I_STORAGE_HPP
#define FUL_SRC_LIB_STORAGE_I_STORAGE_HPP

namespace ful
{

class IStorage
{
public:
    IStorage() = default;
    virtual ~IStorage() = default;

    IStorage(const IStorage&) = default;
    IStorage& operator=(const IStorage&) = default;
    IStorage(IStorage&&) = default;
    IStorage& operator=(IStorage&&) = default;

    virtual void store() = 0;
};

} // namespace ful

#endif // !FUL_SRC_LIB_STORAGE_I_STORAGE_HPP
