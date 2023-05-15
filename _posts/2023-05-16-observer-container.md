---
layout: post
title:  "observer container"
date:   2023-05-16 08:24:58 +0800
categories: jekyll update
tags:
  design-pattern
---

folly中的ObserverContainer实现了观察者模式

    #include <folly/init/Init.h>
    #include <folly/ObserverContainer.h>
    #include <iostream>
    //subject触发的事件
    enum class TestObserverEvents : uint8_t { SpecialEvent = 1, SuperSpecialEvent = 2 };

    //模板参数8代表observerContainer初始reserve的大小
    using TestObserverContainerPolicy = folly::ObserverContainerBasePolicyDefault<TestObserverEvents, 8>;

    //observer的接口
    template <typename ObservedT>
    class TestObserverInterface
    {
     public:
      virtual ~TestObserverInterface() = default;
      virtual void special(ObservedT*) noexcept {}
      virtual void superSpecial(ObservedT*) noexcept {}
      virtual void broadcast(ObservedT*) noexcept {}
    };

    //subject本身，存有ObserverContainer对象
    class TestSubject
    {
     public:
     //构造函数中初始化observercontainer
      TestSubject() : observerCtr(this) {}
      using ObserverContainer =
          folly::ObserverContainer<TestObserverInterface<TestSubject>, TestSubject, TestObserverContainerPolicy>;

      //object special动作触发special事件
      void doSomethingSpecial()
      {
        observerCtr.invokeInterfaceMethod<TestObserverEvents::SpecialEvent>(
            [](auto observer, auto observed) { observer->special(observed); });
      }

      void doSomethingSuperSpecial()
      {
        observerCtr.invokeInterfaceMethod<TestObserverEvents::SuperSpecialEvent>(
            [](auto observer, auto observed) { observer->superSpecial(observed); });
      }

       //object broadcast动作广播触发所有事件。
      void doBroadcast()
      {
        observerCtr.invokeInterfaceMethodAllObservers([](auto observer, auto observed) { observer->broadcast(observed); });
      }

      ObserverContainer observerCtr;
    };

    template <typename ObserverContainerT>
    class ManagedObserverToTestObj : public ObserverContainerT::ManagedObserver
    {
     public:
      using TestSubjectT = typename ObserverContainerT::observed_type;
      using ObserverContainerBase =
          folly::ObserverContainerBase<TestObserverInterface<typename ObserverContainerT::observed_type>,
                                       typename ObserverContainerT::observed_type, TestObserverContainerPolicy>;
      using EventEnum       = typename ObserverContainerT::EventEnum;
      using EventSet        = typename TestSubjectT::ObserverContainer::Observer::EventSet;
      using EventSetBuilder = typename TestSubjectT::ObserverContainer::Observer::EventSetBuilder;

      using ObserverContainerT::ManagedObserver::ManagedObserver;


     private:
     //observer实现各种事件对应的接口
      void special(TestSubjectT* obj) noexcept override { std::cout << "observed special\n"; }
      void superSpecial(TestSubjectT* obj) noexcept override { std::cout << "observed special\n"; }
      void broadcast(TestSubjectT* obj) noexcept override { std::cout << "observed boardcast\n"; }
    };

    void testObserver()
    {
      using MockManagedObserver = ManagedObserverToTestObj<TestSubject::ObserverContainer>;
      {
        std::cout << "----\n";
        //创建一个observer,
           //managedobserver的析构函数会使subject的observerCtr remove掉managedobserber本身，线程不安全
           //一个managedobserver应只观察一个subject,
        auto observer1 =
            std::make_unique<MockManagedObserver>(MockManagedObserver::EventSetBuilder().enableAllEvents().build());
            //注册 observer
        obj1->observerCtr.addObserver(observer1.get());
        //所有的observer的数量
        std::cout << obj1->observerCtr.numObservers() << std::endl;
        //所有的observer的数量
        std::cout << obj1->observerCtr.getObservers().size() << std::endl;
        //所有的observer的数量
        std::cout << obj1->observerCtr.findObservers().size() << std::endl;
        //TestSubject::ObserverContainer::Observer的observer的数量，也就是所有的数量
        std::cout << obj1->observerCtr.findObservers<TestSubject::ObserverContainer::Observer>().size() << std::endl;

        //MockManagedObserver的数量
        std::cout << obj1->observerCtr.findObservers<MockManagedObserver>().size() << std::endl;
        std::cout << "----\n";
        //subject动作，从而触发各种事件
        obj1->doSomethingSuperSpecial();
        obj1->doSomethingSpecial();
        obj1->doBroadcast();
      }
    }

    int main(int argc, char** argv)
    {
      folly::init(&argc, &argv);
      testObserver();
    }
