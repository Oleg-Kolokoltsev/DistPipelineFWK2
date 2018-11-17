//
// Created by morrigan on 2/4/18.
//

#ifndef DISTPIPELINEFWK_COMMAND_NODE_H
#define DISTPIPELINEFWK_COMMAND_NODE_H

#include <string>
#include <memory>

/*
 * Command messages are objects of BaseMessage that
 * have a command field set. Depending from node,
 * it can process or ignore the command.
 *
 * AQUIRE - is used by BaseSource to call func_acquire. This
 * message can be sent from outside world, as in the first tutorial tutorial.
 * Also, it can be sent from the func_acquire itself to synchronize with
 * external data source device and work at it's maximum speed.
 *
 * STOP - is processed by BaseNode and used to terminate node thread.
 * This message is processed by all nodes.
 *
 * NONE - message field is not set, used in user data messages
 *
 * USER - is used to specify (IClonable*) user_data message field,
 * that can be processed by the dedicated node (see IClonable and
 * dsp/dfrft_filter for an example).
 */
enum class MSG_CMD{ACQUIRE, STOP, NONE, USER};

/*
 * Node node-to-node chain policies. Each node
 * has an input buffer. When this buffer is full,
 * the node that sends the message can drop it
 * or wait until the receiver process it's previous
 * messages.
 *
 * DROP - is used to process the most recent data
 * WAIT - is used to speed down sender and avoid data loss
 */
enum class QUEUE_POLICY{WAIT, DROP};

class NodeFactory;
class ICloneable;

struct CommandNode{
    CommandNode(std::string name): name{name}{};

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool put(MSG_CMD, unsigned int sent_from, QUEUE_POLICY, std::shared_ptr<ICloneable>&& ) = 0;

    // used to down-cast pointers from CommandNode* to ConcreteNode*
    virtual ~CommandNode() {};

protected:
    // unique identifier of the node
    unsigned int uid;

    // a node name is not used in algorithms and
    // can be safely used for debug purposes
    std::string name;

private:
    // the UID can be set by NodeFactory only
    friend class NodeFactory;
    void set_uid(unsigned int uid){this->uid = uid;}

public:
    unsigned int get_uid(){return uid;}
    std::string get_name(void){return name;}
};

#endif //DISTPIPELINEFWK_COMMAND_NODE_H
