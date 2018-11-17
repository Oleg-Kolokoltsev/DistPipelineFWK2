//
// Created by morrigan on 12/01/18.
//

#ifndef DISTPIPELINEFWK_CMD_DATA_TYPES_H
#define DISTPIPELINEFWK_CMD_DATA_TYPES_H

#include <memory>

#include "command_node.h"

/*
 * It is possible that one or more nodes would require to
 * transmit control message(s) to any other node(s).
 * In this case, the tBaseMessage struct can contain
 * a not null cmd_data field for specific information carried with
 * the command.
 *
 * Each transmitted data shell implement the ICmdData interface to let
 * another nodes, such as "simple_splitter", to copy that data
 * among multiply channels.
 */


/*
 * WARNING!
 *
 * This is the "clone pattern". It is required to copy all command
 * data in a generic way. The "clone" function need to be virtual
 * and reimplemented at any level of inheritance. An example below
 * will produce a hidden bug:
 *
 * //the clone function works fine
 * class A : public ICloneable{
 *   virtual std::shared_ptr<ICloneable> clone() const{
 *      return std::shared_ptr<A>(new A(*this));
 *   };
 * }
 *
 * //the A class clone function will be used
 * but child data will be lost.
 * class B : public A{
 * }
 */

struct ICloneable{
    using tPtrCloneable = std::shared_ptr<ICloneable>;

    // override 'clone' in EACH child at any level of inheritance!
    virtual tPtrCloneable clone() const = 0;

    /*
     * The destination node object will call this function to apply command
     * message on itself in a thread safe manner. Parameter is a pointer to
     * receiver node object.
     */
    virtual void apply(CommandNode* ) = 0;
};

/*
 * The broadcast command message structure shell be clear for all nodes,
 * independently if they use it or not. Therefore, all broadcasts has it's
 * unique MSG_CMD identifier, and for each of these, their CmdData structure
 * is defined below to be visible by all node implementations.
 */

#endif //DISTPIPELINEFWK_CMD_DATA_TYPES_H
