/**
 * Commons specific to the Advanced Computer Graphics class
 */

// whenever we try to include it, we remove any group definition
#ifdef GROUP_NUMBER
#undef GROUP_NUMBER
#endif

#ifndef ACG_H
#define ACG_H
// concatenation with correct macro expansion
#include <boost/preprocessor/cat.hpp>
// @see http://stackoverflow.com/questions/18216019/expansion-of-c-macro-within-macro-to-be-concatenated
// class name for each group
#define CLASS_NAME(cls, groupId) BOOST_PP_CAT(cls, groupId)
// class name stringified
#define MACRO_STR_IMPL(x) #x
#define MACRO_STRING(x) MACRO_STR_IMPL(x)
#define CLASS_STRING_IMPL(cls, groupId) MACRO_STRING(BOOST_PP_CAT(cls, groupId))
#define CLASS_STRING(cls, groupId) CLASS_STRING_IMPL(cls, groupId)

// group namespace functions
#define GROUP_NAMESPACE_BEGIN_IMPL(ns) namespace ns {
#define GROUP_NAMESPACE_BEGIN_FUNC(g) GROUP_NAMESPACE_BEGIN_IMPL(g)
#define GROUP_NAMESPACE_END }
#define GROUP_NAMESPACE() BOOST_PP_CAT(group_, GROUP_NUMBER)
#define GROUP_NAMESPACE_BEGIN() GROUP_NAMESPACE_BEGIN_FUNC(GROUP_NAMESPACE())
// group class using namespace
#define GROUP_CLASS_IMPL(cls, g) group_##g::cls
#define GROUP_CLASS(cls, g) GROUP_CLASS_IMPL(cls, g)
// group class without namespace
#define GROUP_CLASS0_IMPL(cls, g) group_##g##_##cls
#define GROUP_CLASS0(cls, g) GROUP_CLASS0_IMPL(cls, g)

// nori registration
#include <nori/object.h>
#define NORI_REGISTER_GROUP_CLASS_IMPL(cls, cls0, name, ver) NORI_REGISTER_CLASS_VERSION(cls, cls0, name, ver)
#define NORI_REGISTER_GROUP_CLASS(cls, name) NORI_REGISTER_GROUP_CLASS_IMPL(GROUP_CLASS(cls, GROUP_NUMBER), GROUP_CLASS0(cls, GROUP_NUMBER), name, GROUP_NUMBER)
#endif
