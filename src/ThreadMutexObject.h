/*
 * ThreadMutexObject.h
 *
 *  Created on: 11 May 2012
 *      Author: thomas
 */

#ifndef THREADMUTEXOBJECT_H_
#define THREADMUTEXOBJECT_H_

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition_variable.hpp>

template <class T>
class ThreadMutexObject
{
    public:
        ThreadMutexObject()
        {}

        ThreadMutexObject(T initialValue)
         : object(initialValue)
        {}

        void assignValue(T newValue)
        {
            boost::mutex::scoped_lock lock(mutex);

            object = newValue;

            lock.unlock();
        }

        boost::mutex & getMutex()
        {
            return mutex;
        }

        T & getReference()
        {
            return object;
        }

        void assignAndNotifyAll(T newValue)
        {
            boost::mutex::scoped_lock lock(mutex);

            object = newValue;

            signal.notify_all();

            lock.unlock();
        }
        
        void notifyAll()
        {
            boost::mutex::scoped_lock lock(mutex);

            signal.notify_all();

            lock.unlock();
        }

        T getValue() const
        {
            boost::mutex::scoped_lock lock(mutex);

            T lastCopy = object;

            lock.unlock();

            return lastCopy;
        }

        T waitForSignal()
        {
            boost::mutex::scoped_lock lock(mutex);

            signal.wait(mutex);

            T lastCopy = object;

            lock.unlock();

            return lastCopy;
        }

        T getValueWait(int wait = 33000)
        {
            boost::this_thread::sleep(boost::posix_time::microseconds(wait));

            boost::mutex::scoped_lock lock(mutex);

            T lastCopy = object;

            lock.unlock();

            return lastCopy;
        }

        void operator++(int)
        {
            boost::mutex::scoped_lock lock(mutex);

            object++;

            lock.unlock();
        }

    private:
        T object;
        mutable boost::mutex mutex;
        boost::condition_variable_any signal;
};

#endif /* THREADMUTEXOBJECT_H_ */
