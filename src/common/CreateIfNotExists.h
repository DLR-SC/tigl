#ifndef CREATEIFNOTEXISTSTAG_H
#define CREATEIFNOTEXISTSTAG_H

namespace tigl
{
    // define CreateIfNotExistsTag as a type which's values are compile-time constants
    struct CreateIfNotExistsTag {};
    static const CreateIfNotExistsTag CreateIfNotExists = CreateIfNotExistsTag();
}

#endif
