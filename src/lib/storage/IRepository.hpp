#ifndef FUL_SRC_LIB_STORAGE_I_STORAGE_HPP
#define FUL_SRC_LIB_STORAGE_I_STORAGE_HPP

namespace ful
{

class IRepository
{
public:
    IRepository() = default;
    virtual ~IRepository() = default;

    IRepository(const IRepository&) = default;
    IRepository& operator=(const IRepository&) = default;
    IRepository(IRepository&&) = default;
    IRepository& operator=(IRepository&&) = default;

    virtual void store() = 0;
};

} // namespace ful

#endif // !FUL_SRC_LIB_STORAGE_I_STORAGE_HPP
