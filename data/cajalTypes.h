#ifndef CAJAL_TYPES_H
#define CAJAL_TYPES_H

#include <vector>
#include <string>

//! A Trace node argument. It only has semantic meaning
typedef std::string NodeArg;
//! The value of a node argument
typedef std::string ParamValue;
typedef std::pair<NodeArg, ParamValue> NodeParam;
typedef std::vector<NodeParam> NodeParamList;


#endif// CAJAL_TYPES_H