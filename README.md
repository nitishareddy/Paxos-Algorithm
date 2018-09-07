# Paxos-Algorithm

A. Problem Definition:
In this project you will simulate a simple distributed consensus problem using network socket programming. We will spawn different processes and try to model the problem.
Paxos is a popular fault-tolerant distributed consensus algorithm. It allows a globally consistant (total) order to be assigned to client messages (actions).
Much of what is summarized here is from Lamport's Paxos Made Simple but I tried to simplify it substantially. Please refer to that paper for more detail and definitive explanations.
  
The goal of a distributed consensus algorithm is to allow a set of computers to all agree on a single value that one of the nodes in the system proposed (as opposed to making up a random value). The challenge in doing this in a distributed system is that messages can be lost or machines cn fail. Paxos guarantees that a set of machines will chose a single proposed value as long as a majority of systems that participate in the algorithm are available.
The setting for the algorithm is that of a collection of processes that can propose values. The algorithm must ensure that a single one of those proposed values is chosen and all processes should learn that value.
There are three classes of agents:
1. Proposers 2. Acceptors 3. Learners
A machine can take on any or all these roles.Proposersput forth proposed values. Acceptors drive the algorithm's goal to reach agreement on a single value and let the learners are informed of the outcome. Acceptors either reject a proposal or agree to it and make promises on what proposals they will accept in the future. This ensures that only the latest set of proposals will be accepted. A process can act as more than one agent in an implementation. Indeed, many implementations have collections of processes where each process takes on all three roles.
Agents communicate with each other asynchronously. They may also fail to communicate and may restart. Messages can take arbitrarily long to deliver. They can can be duplicated or lost but are not corrupted. A corrupted message should be detectable as such and can be counted as a lost one (this is what UDP does, for example).
The simplest implementation contains a single acceptor. A proposer sends a proposal value to the acceptor. The acceptor processes one request at a time, chooses the first proposed value that it receives, and lets everyone (learners) know. Other proposers must agree to that value.
This works if the acceptor doesn't fail. Unfortunately, acceptors are subject to failure. To guard against the failure of an acceptor, we turn to replication and use multiple acceptor processes. A proposer now sends a proposal containing a value to a set of acceptors. The value is chosen when a majority of the acceptors accept that proposal (agree to it).
Different proposers, however, could independently initiate proposals at approximately the same time and those proposals could contain different values. They each will communicate with a different subset of acceptors. Now different acceptors will each have different values but none will have a majority. We need to allow an acceptor to be able to accept more than

a. b.
one proposal. We will keep track of proposals by assigning a unique proposal number to each proposal. Each proposal will contain a proposal number and a value. Different proposals must have different proposal numbers. Our goal is to agree on one of those proposed values from the pool of proposals sent to different subsets of acceptors.
A value is chosen when a single proposal with that value has been accepted by a majority of the acceptors. That means it has been chosen. Multiple proposals can be chosen but all of them bust have the same value: if a proposal with a value v is chosen, then every higher- numbered proposal that is chosen must also have value v.
If a proposal with proposal number n and value v is issued, then there is a set S consisting of a majority of acceptors such that either:
no acceptor in S has accepted any proposal numbered less than n, or
v is the value of the highest-numbered proposal among all proposals numbered < n accepted by the acceptors in S.
A proposer that wants to issue a proposal numbered n must learn the highest numbered proposal with number less than n, if any, that has been or will be accepted by each acceptor in a majority of acceptors. To do this, the proposer gets a promisefrom an acceptor that there will be no future acceptance of proposals numbered less than n.
The Paxos algorithm operates in two phases:
Phase 1: Prepare: send a proposal request
Proposer:
A proposer chooses a proposal number n and sends a prepare request to a majority of acceptors. The number n is stored in the proposer's stable storage so that the proposer can ensure that a higher number is used for the next proposal (even if the proposer process restarts).
Acceptor:
▪ If an acceptor has received a proposal greater than n in the past, then it ignores this prepare(n) request.
▪ The acceptor promises never to accept a proposal numbered less than n.
▪ The acceptor replies to the proposer with a past proposal that it has accepted
previously that had the highest number less than n: reply(n',v').
If a proposer receives the requested responses to its prepare request from a majority of the acceptors, then it can issue a proposal with number n and value v, where v is the value of the highest-numbered proposal among the responses or any value selected by the proposer if the responding acceptors reported no proposals.
Phase 2: Accept: send a proposal (and then propagate it to learners after acceptance)

Proposer:
A proposer can now issue its proposal. It will send a message to a set of acceptors stating that its proposal should be accepted (an accept(n,v) message). If the proposer receives a response to its prepare(n) requests from a majority of acceptors, it then sends an accept(n, v) request to each of those acceptors for a proposal numbered n with a value v, where v is the highest-numbered proposal among the responses, or is any value if the responses reported no proposals.
Acceptor:
If an acceptor receives an accept(n, v) request for a proposal numbered n, it accepts the proposal unless it has already responded to a prepare request having a number greater than n.
The acceptor receives two types of requests from proposers: prepare and accept requests. Any request can be ignored. An acceptor only needs to remember the highest-numbered proposal that it has ever accepted and the number of the highest-numbered prepare request to which it has responded. The acceptor must store these values in stable storage so they can be preserved in case the acceptor fails and has to restart.
A proposer can make multiple proposals as long as it follows the algorithm for each one.
Now that the acceptors have a proposed value, we need a way to learn that a proposal has been accepted by a majority of acceptors. The learner is responsible for getting this information. Each acceptor, upon accepting a proposal, forwards it to all the learners. The problem with doing this is the potentially large number of duplicate messages: (number of acceptors) * (number of learners). If desired, this could be optimized. One or more "distinguished learners" could be elected. Acceptors will communicate to them and they, in turn, will inform the other learners.
One problem with the algorithm is that it’s possible for two proposers to keep issuing sequences of proposals with increasing numbers, none of which get chosen. An accept message from one proposer may be ignored by an acceptor because a higher numbered prepare message has been processed from the other proposer. To ensure that the algorithm will make progress, a "distinguished proposer" is selected as the only one to try issuing proposals. You needn’t implement this in your lab.
