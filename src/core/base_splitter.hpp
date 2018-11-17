//
// Created by Dr, Yevgeniy Kolokoltsev on 1/30/17.
//

#ifndef COMPPHYSFWK_PIPELINE_HPP
#define COMPPHYSFWK_PIPELINE_HPP

#include "base_node.hpp"

/*
 * todo:
 * To generate UIDs and keep them unique it is used a singleton
 * class UIDGen. It keeps all UIDs and generate a random one for each class.
 */

//todo: kill this alias
using tPtrCond = std::shared_ptr<std::condition_variable>;

/*
 * todo: Describe policies and typical use cases, when and which policy shell be used
 */

//************************SPLITTER***********************
template<typename tIn>
class BaseSplitter : public BaseNode<tIn>{

public:
    using tBase = BaseNode<tIn>;
    using tPtrIn = std::shared_ptr<tIn>;
    using tPtrNext = std::shared_ptr<BaseNode<tIn>>;

    /*
     * If output policy is DROP, the packets will
     * not synchronize between the output channels, that can be important
     * in the case of parallel computations.
     */
protected:
    friend class NodeFactory;
    BaseSplitter(QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "BaseSplitter"):
            tBase(name),
            pol{pol} {}

    virtual bool process_usr_msg(tPtrIn&& msg){
        if(!targets.empty()){
            for(auto it = targets.begin(); it != targets.end(); it++) {
                bool ret_val;
                if(it != --targets.end()){
                    auto cpy_msg = tPtrIn(new typename tPtrIn::element_type(*msg));
                    ret_val = (*it)->put(move(cpy_msg), this->uid, pol);
                }else{
                    ret_val = (*it)->put(move(msg), this->uid, pol);
                }
                if (!ret_val)
                    std::cerr << tBase::name << " warning: target \"" << (*it)->get_name()
                              << "\" failed to receive a message" << std::endl;
            }
            return true;
        }else{
            std::cerr << tBase::name << " broken pipe" << std::endl;
            return false;
        }
    };

public:
    void add_target(tPtrNext target){targets.push_back(target);}

private:
    QUEUE_POLICY pol;

protected:
    std::list<tPtrNext> targets;
};

#endif //COMPPHYSFWK_PIPELINE_HPP
