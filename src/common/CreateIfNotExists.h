#ifndef CREATEIFNOTEXISTSTAG_H
#define CREATEIFNOTEXISTSTAG_H

namespace tigl
{
    // define CreateIfNotExistsTag as a type which's values are compile-time constants (see implementation of boost::none)
    namespace internal
    {
        struct CreateIfNotExistsTagHelper{};
    }
    typedef int internal::CreateIfNotExistsTagHelper::* CreateIfNotExistsTag;

    // create constant CreateIfNotExists
    const CreateIfNotExistsTag CreateIfNotExists = static_cast<CreateIfNotExistsTag>(0);
}

#endif