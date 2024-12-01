#include "tests.h"
#include "tests_helper.h"
#include <tweeny/tweeny.h>
#include <tweeny/detail/tweeny_internal.h>
#include <suitepp/suite.hpp>

namespace tweeny
{

int TWEENY_UPDATE_STEP_COUNT = 10;
tweeny::ease_t EASING = tweeny::ease::linear;

inline void tweeny_update(tweeny::duration_t duration)
{   
    //test update with 0
    tweeny::update(tweeny::duration_t::zero());
    auto step = duration / TWEENY_UPDATE_STEP_COUNT;
    for(int i = 0; i < TWEENY_UPDATE_STEP_COUNT; ++i)
    {
        tweeny::update(step);
    }
}

template<typename T>
struct values_t
{
    values_t() = default;
    values_t(T val, bool use_shared)
    {
        ptr = std::make_shared<T>(val);
        value = val;
        sentinel = std::make_shared<T>(val);;
        use_shared_ptr = use_shared;
    }
    T& get_value()
    {
        return use_shared_ptr ? *ptr : value;
    }

    const T& get_value() const
    {
        return use_shared_ptr ? *ptr : value;
    }

    void sentinel_reset()
    {
        use_shared_ptr ? ptr.reset() : sentinel.reset();
    }

    bool sentinel_expired() const
    {
        return use_shared_ptr ? ptr == nullptr : sentinel == nullptr;
    }

    std::shared_ptr<T> ptr{};
    T value{};
    std::shared_ptr<void> sentinel{};
    bool use_shared_ptr{};
};


template<typename T>
void core_tween_test_impl(tweeny::tween_action& action,
                          tweeny::duration_t duration,
                          bool step_update,
                          const values_t<T>& values,
                          const T& begin,
                          const T& end)
{
    THEN("the action should be valid")
    {
        REQUIRE( action.is_valid() );
    };

    WHEN("the action is paused before starting")
    {
        //pause before starting should do nothing
        REQUIRE_NOTHROWS(tweeny::pause(action));
    };

    THEN("it should not be paused")
    {
        REQUIRE(!tweeny::is_paused(action));
    };

    WHEN("the action is resumed before starting")
    {
        REQUIRE_NOTHROWS(tweeny::resume(action));
    };

    THEN("it should not be running")
    {
        REQUIRE(!tweeny::is_running(action));
    };

    AND_WHEN("the tween action is started")
    {
        REQUIRE_NOTHROWS(tweeny::start(action));
    };

    THEN("if the tweeny is running")
    {
        //pause after starting successfully should work just fine
        if(tweeny::is_running(action))
        {
            REQUIRE_NOTHROWS(tweeny::set_speed_multiplier(action, 10.0f));
            REQUIRE(int(tweeny::get_speed_multiplier(action)) == int(10.0f));
            REQUIRE(tweeny::get_duration(action) > 0s);
            REQUIRE(tweeny::get_elapsed(action) == 0s);

            REQUIRE_NOTHROWS(tweeny::pause(action));
            REQUIRE(tweeny::is_paused(action));

            REQUIRE_NOTHROWS(tweeny::resume(action));
            REQUIRE(tweeny::is_running(action));
            REQUIRE(!tweeny::is_finished(action));
        }
    };

    WHEN("'stop_when_finished' is called")
    {
        REQUIRE_NOTHROWS(tweeny::stop_when_finished(action));
    };
    THEN("the action is 'stopping'")
    {
        REQUIRE(tweeny::is_stopping(action));
    };

    if(duration > 0s)
    {
        if(!values.sentinel_expired())
        {
            if(tweeny::is_running(action))
            {
                WHEN("duration > 0 and the sentinel is NOT expierd and the action is running")
                {
                };
                AND_WHEN("the sentinel is NOT expierd")
                {
                };
                AND_WHEN("the action is running")
                {
                };
                THEN("the object should be equal to the begin value")
                {
                    auto result = helper::compare(values.get_value(), begin);
                    if(!result)
                    {
                        REQUIRE(values.get_value() == begin);
                    }
                };
            }
        }
    }

    if(step_update)
    {
        WHEN("'update' is called")
        {
            REQUIRE_NOTHROWS(tweeny_update(duration));
        };
    }
    else
    {
        WHEN("'stop_and_finish' is called")
        {
            REQUIRE_NOTHROWS(tweeny::stop_and_finish(action));
        };
    }

    THEN("there should be no lingering tweenies internally")
    {
        REQUIRE_NOTHROWS(tweeny::detail::get_manager().get_tweenies().empty());
    };

    if(duration > 0s && tweeny::get_elapsed(action) >= duration)
    {
        if(!values.sentinel_expired())
        {
            WHEN("duration > 0 and the action actually finished its duration")
            {

            };

            AND_WHEN("the sentinel is not expired")
            {

            };

            THEN("the object should be equal to the end value")
            {
                auto result = helper::compare(values.get_value(), end);
                if(!result)
                {
                    REQUIRE(values.get_value() == end);
                }
            };
        }
    }
}


template<typename T>
tweeny::tween_action creator(const std::string& type,
                             values_t<T>& values,
                             T& begin,
                             const T& end,
                             tweeny::duration_t duration)
{
    if(type == "tween_from_to")
    {
        if(values.use_shared_ptr)
        {
            return tweeny::tween_from_to(values.ptr, begin, end, duration, EASING);
        }
        return tweeny::tween_from_to(values.value, begin, end, duration, values.sentinel, EASING);
    }

    if(type == "tween_to")
    {
        if(!values.sentinel_expired())
        {
            begin = values.get_value();
        }

        if(values.use_shared_ptr)
        {
            return tweeny::tween_to(values.ptr, end, duration, EASING);
        }
        return tweeny::tween_to(values.value, end, duration, values.sentinel, EASING);
    }

    if(type == "tween_by")
    {
        if(!values.sentinel_expired())
        {
            begin = values.get_value();
        }

        if(values.use_shared_ptr)
        {
            return tweeny::tween_by(values.ptr, end, duration, EASING);
        }
        return tweeny::tween_to(values.value, end, duration, values.sentinel, EASING);
    }

    return {};
}
template< typename T>
void scenario(bool use_shared_ptr,
              bool step_update,
              const std::string& type,
              tweeny::duration_t duration,
              T begin,
              T end,
              T object_value)
{

    {
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        auto record_input = [&]()
        {
             GIVEN( "object == " + suite::helper::to_string(object_value) ) {};
             GIVEN( "begin == " + suite::helper::to_string(begin) ) {};
             GIVEN( "end == " + suite::helper::to_string(end) ) {};
             GIVEN( "duration == " + suite::helper::to_string(duration_ms) + "ms" ) {};
             GIVEN( "step_update == " + suite::helper::to_string(step_update)) {};
        };

        SCENARIO("a valid sentinel is passed")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should finish successfully")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("an invalid sentinel is passed")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and an invalid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
                values.sentinel_reset();
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            THEN("the action should not be valid")
            {
                REQUIRE( !action.is_valid() );
            };
        };

        SCENARIO("the 'on_begin' callback expires the sentinel")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            AND_WHEN("an 'on_begin' callback is connected")
            {
                action.on_begin.connect([&values]()
                                        {
                                            values.sentinel_reset();
                                        });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be finished and the sentinel should be expired")
            {
                REQUIRE( tweeny::is_finished(action) );
                REQUIRE( values.sentinel_expired() );
            };
        };

        SCENARIO("the 'on_begin' callback calls 'stop'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            AND_WHEN("an 'on_begin' callback is connected")
            {
                action.on_begin.connect([&action]()
                                        {
                                            REQUIRE_NOTHROWS(tweeny::stop(action));
                                        });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should finish successfully")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_begin' callback calls 'stop_and_finish'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            AND_WHEN("an 'on_begin' callback is connected")
            {
                action.on_begin.connect([&action]()
                                        {
                                            REQUIRE_NOTHROWS(tweeny::stop_and_finish(action));
                                        });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should finish successfully")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_begin' callback calls 'pause'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            AND_WHEN("an 'on_begin' callback is connected")
            {
                action.on_begin.connect([&action]()
                                        {
                                            REQUIRE_NOTHROWS(tweeny::pause(action));
                                        });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            if(step_update)
            {
                THEN("the action should be paused")
                {
                    REQUIRE( tweeny::is_paused(action) );
                };
            }
            else
            {
                THEN("the action should be finished")
                {
                    REQUIRE( tweeny::is_finished(action) );
                };
            }
        };


        SCENARIO("the 'on_update' callback expires the sentinel")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            bool is_sentinel_reset_requested = false;

            AND_WHEN("an 'on_update' callback is connected")
            {
                action.on_update.connect([&]()
                                         {
                                             is_sentinel_reset_requested = true;
                                             values.sentinel_reset();
                                         });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
            if(is_sentinel_reset_requested)
            {
                REQUIRE( values.sentinel_expired() );
            }
        };

        SCENARIO("the 'on_update' callback calls 'stop'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            AND_WHEN("an 'on_update' callback is connected")
            {
                action.on_update.connect([&action]()
                                         {
                                             REQUIRE_NOTHROWS(tweeny::stop(action));
                                         });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_update' callback calls 'stop_and_finish'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            AND_WHEN("an 'on_update' callback is connected")
            {
                action.on_update.connect([&action]()
                                         {
                                             REQUIRE_THROWS_AS(tweeny::stop_and_finish(action), std::runtime_error);
                                         });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_update' callback calls 'pause'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            bool is_pause_requested = false;
            AND_WHEN("an 'on_update' callback is connected")
            {
                action.on_update.connect([&]()
                                         {
                                             is_pause_requested = true;
                                             REQUIRE_NOTHROWS(tweeny::pause(action));
                                         });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            if(is_pause_requested && step_update)
            {
                THEN("the action should be paused")
                {
                    REQUIRE( tweeny::is_paused(action) );
                };
            }
            else
            {
                THEN("the action should be is_finished")
                {
                    REQUIRE( tweeny::is_finished(action) );
                };
            }
        };

        SCENARIO("the 'on_step' callback expires the sentinel")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            bool is_sentinel_reset_requested = false;

            AND_WHEN("an 'on_step' callback is connected")
            {
                action.on_step.connect([&]()
                                       {
                                           is_sentinel_reset_requested = true;
                                           values.sentinel_reset();
                                       });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
            if(is_sentinel_reset_requested)
            {
                REQUIRE( values.sentinel_expired() );
            }
        };

        SCENARIO("the 'on_step' callback calls 'stop'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            AND_WHEN("an 'on_step' callback is connected")
            {
                action.on_step.connect([&]()
                                       {
                                           REQUIRE_NOTHROWS(tweeny::stop(action));
                                       });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_step' callback calls 'stop_and_finish'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            AND_WHEN("an 'on_step' callback is connected")
            {
                action.on_step.connect([&action, counter = 0]() mutable
                                       {
                                           //first on_step can be called from start();
                                           if(counter++ > 0)
                                           {
                                               REQUIRE_THROWS_AS(tweeny::stop_and_finish(action), std::runtime_error);
                                           }
                                       });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_step' callback calls 'pause'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            bool is_pause_requested = false;

            AND_WHEN("an 'on_step' callback is connected")
            {
                action.on_step.connect([&]()
                                       {
                                           is_pause_requested = true;
                                           REQUIRE_NOTHROWS(tweeny::pause(action));
                                       });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            if(is_pause_requested && step_update)
            {
                THEN("the action should be is_paused")
                {
                    REQUIRE( tweeny::is_paused(action) );
                };
            }
            else
            {
                THEN("the action should be is_finished")
                {
                    REQUIRE( tweeny::is_finished(action) );
                };
            }
        };

        SCENARIO("the 'on_end' callback expires the sentinel")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            bool is_sentinel_reset_requested = false;

            AND_WHEN("an 'on_end' callback is connected")
            {
                action.on_end.connect([&]()
                                      {
                                          is_sentinel_reset_requested = true;
                                          values.sentinel_reset();
                                      });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };

            if(is_sentinel_reset_requested)
            {
                REQUIRE( values.sentinel_expired() );
            }
        };

        SCENARIO("the 'on_end' callback calls 'stop'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            AND_WHEN("an 'on_end' callback is connected")
            {
                action.on_end.connect([&action]()
                                      {
                                          REQUIRE_NOTHROWS(tweeny::stop(action));
                                      });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_end' callback calls 'stop_and_finish'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };
            AND_WHEN("an 'on_end' callback is connected")
            {
                action.on_end.connect([&action]()
                                      {
                                          REQUIRE_NOTHROWS(tweeny::stop_and_finish(action));
                                      });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };

        SCENARIO("the 'on_end' callback calls 'pause'")
        {
            record_input();

            values_t<T> values;
            GIVEN("a valid object and a valid sentinel")
            {
                values = values_t<T>(object_value, use_shared_ptr);
            };

            tweeny::tween_action action{};
            WHEN("the tween action is created")
            {
                action = creator(type, values, begin, end, duration);
            };

            AND_WHEN("an 'on_end' callback is connected")
            {
                action.on_end.connect([&action]()
                                      {
                                          REQUIRE_NOTHROWS(tweeny::pause(action));
                                      });
            };

            core_tween_test_impl(action, duration, step_update, values, begin, end);

            THEN("the action should be is_finished")
            {
                REQUIRE( tweeny::is_finished(action) );
            };
        };
    };
}


template<typename T>
void core_tween_test(const std::string& type,
                     const std::string& easing_type,
                     tweeny::duration_t duration,
                     T begin,
                     T end,
                     T object_value)
{
    TEST_GROUP( type +"<" + tweeny::inspector::type_to_str(T{}) + "> with easing [" + easing_type + "]" )
    {

        TEST_GROUP( "with a value" )
        {
            scenario(false, true, type, duration, begin, end, object_value);
        };

        TEST_GROUP( "with a value" )
        {
            scenario(false, true, type, duration, begin, end, object_value);
        };

        TEST_GROUP( "with a shared_ptr" )
        {
            scenario(true, true, type, duration, begin, end, object_value);
        };

        TEST_GROUP( "with a shared_ptr" )
        {
            scenario(true, false, type, duration, begin, end, object_value);
        };
    };
}

template<typename T>
void run_tween_test(const std::string& type,
               const std::string& easing_type,
               const T& begin,
               const T& end,
               const T& object)
{
    core_tween_test<T>(type, easing_type, helper::random_value(-1000s, -1s), begin, end, object);
    core_tween_test<T>(type, easing_type, 0s, begin, end, object);
    core_tween_test<T>(type, easing_type, helper::random_value(1s, 1000s), begin, end, object);
}

void test_scopes()
{
    tweeny::scope::push("test1");
    {
        auto t1 = tweeny::start(tweeny::delay(1s));
        auto t2 = tweeny::start(tweeny::delay(1s));
        tweeny::scope::pause_all("test1");
        REQUIRE(tweeny::is_paused(t1));
        REQUIRE(tweeny::is_paused(t2));
        tweeny::scope::resume_all("test1");
        REQUIRE(!tweeny::is_paused(t1));
        REQUIRE(!tweeny::is_paused(t2));
        tweeny::scope::stop_and_finish_all("test1");
        REQUIRE(tweeny::is_finished(t1));
        REQUIRE(tweeny::is_finished(t2));
    }

    {
        tweeny::scope::push("test2");
        auto t1 = tweeny::start(tweeny::delay(1s));
        auto t2 = tweeny::start(tweeny::delay(1s));
        tweeny::scope::push("test3");
        auto t3 = tweeny::start(tweeny::delay(1s));
        auto t4 = tweeny::start(tweeny::delay(1s));

        tweeny::scope::stop_and_finish_all("test1");
        REQUIRE(tweeny::is_finished(t1));
        REQUIRE(tweeny::is_finished(t2));
        REQUIRE(tweeny::is_finished(t3));
        REQUIRE(tweeny::is_finished(t4));
    }

    REQUIRE(tweeny::scope::get_current() == "test3");
    tweeny::scope::pop();
    REQUIRE(tweeny::scope::get_current() == "test2");
    tweeny::scope::close("test1");
    REQUIRE(tweeny::scope::get_current().empty());

    REQUIRE_THROWS(tweeny::scope::pop());
    tweeny::scope::push("test1");
    tweeny::scope::push("test2");
    tweeny::scope::push("test3");
    tweeny::scope::push("test4");
    REQUIRE(tweeny::scope::get_current() == "test4");
    tweeny::scope::pop();
    REQUIRE(tweeny::scope::get_current() == "test3");

    REQUIRE_THROWS_AS(tweeny::scope::push("test2"), std::logic_error);
    tweeny::scope::clear();

    {
        //stacked
        tweeny::scope::push("test1");
        tweeny::scope::push("test2");
        auto t1 = tweeny::start(tweeny::delay(100s), "test3");
        REQUIRE(tweeny::scope::get_current() == "test2");
        REQUIRE(tweeny::has_tween_with_scope("test3"));
        tweeny::scope::stop_and_finish_all("test2");
        REQUIRE(tweeny::is_finished(t1));

        tweeny_update(1s); // process for tweens pending to remove.
        REQUIRE(!tweeny::has_tween_with_scope("test3"));

        tweeny::scope::clear();
    }

    {
        tweeny::tween_scope_policy scope_policy;
        scope_policy.scope = "test3";
        scope_policy.policy = tween_scope_policy::policy_t::independent;

        tweeny::scope::push("test1");
        tweeny::scope::push("test2");

        auto t1 = tweeny::start(tweeny::delay(100s), scope_policy);
        REQUIRE(tweeny::scope::get_current() == "test2");
        REQUIRE(tweeny::has_tween_with_scope("test3"));
        tweeny::scope::stop_and_finish_all("test2");
        tweeny::scope::close("test2");

        tweeny_update(1s); // process for tweens pending to remove.

        REQUIRE(tweeny::scope::get_current() == "test1");
        REQUIRE(tweeny::is_running(t1));
        REQUIRE(tweeny::has_tween_with_scope("test3"));
        tweeny::stop_and_finish(t1);
    }

    {
        tweeny::scope::clear();
        REQUIRE(tweeny::scope::get_current().empty());
        tweeny::scope::push("test1");
        auto t1 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test1"));

        tweeny::scope::push("test2");
        auto t2 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test2"));

        tweeny::scope::push("test3");
        auto t3 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test3"));

        tweeny::scope::push("test4");
        auto t4 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test4"));

        tweeny::scope::pop();
        REQUIRE(tweeny::scope::get_current() == "test3");
        tweeny::scope::stop_all("test1");

        tweeny_update(1s); // process for tweens pending to remove.

        REQUIRE(!tweeny::has_tween_with_scope("test1"));
        REQUIRE(!tweeny::has_tween_with_scope("test2"));
        REQUIRE(!tweeny::has_tween_with_scope("test3"));
        REQUIRE(!tweeny::has_tween_with_scope("test4"));
    }

    {
        tweeny::scope::clear();
        REQUIRE(tweeny::scope::get_current().empty());

        tweeny::scope::push("test1");
        auto t1 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test1"));

        tweeny::scope::push("test2");
        auto t2 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test2"));

        tweeny::scope::push("test3");
        auto t3 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test3"));

        tweeny::scope::push("test4");
        auto t4 = tweeny::start(tweeny::delay(100s));
        REQUIRE(tweeny::has_tween_with_scope("test4"));

        tweeny::scope::pause_all("test1", "KEY");
        REQUIRE(tweeny::is_paused(t1));
        REQUIRE(tweeny::is_paused(t2));
        REQUIRE(tweeny::is_paused(t3));
        REQUIRE(tweeny::is_paused(t4));

        tweeny::resume(t1);
        REQUIRE(tweeny::is_paused(t1));
        tweeny::resume(t2);
        REQUIRE(tweeny::is_paused(t2));
        tweeny::resume(t3);
        REQUIRE(tweeny::is_paused(t3));
        tweeny::resume(t4);
        REQUIRE(tweeny::is_paused(t4));

        tweeny::scope::resume_all("test1", "KEY");

        REQUIRE(!tweeny::is_paused(t1));
        REQUIRE(!tweeny::is_paused(t2));
        REQUIRE(!tweeny::is_paused(t3));
        REQUIRE(!tweeny::is_paused(t4));


    }
}

void run(bool use_random_inputs)
{
//    suite::get_test_label_matcher() = "*[155724]";
    const auto& ease_list = ease::get_ease_list();
//    for(const auto& kvp : ease_list)
    {
        const auto& kvp = ease_list.front();
        const auto& easing_type = kvp.first;
        EASING = kvp.second;

        hpp::for_each_type
        <
            uint8_t,// uint16_t, uint32_t, uint64_t,
            int8_t,// int16_t, int32_t, int64_t,
            float//, double
        >
        ([&](const auto& el)
        {
            using T = typename std::decay_t<decltype(el)>::type;

            auto object = helper::random_value<T>();
            auto begin = std::numeric_limits<T>::min() / 2;
            auto end = std::numeric_limits<T>::max() / 2;

            if(use_random_inputs)
            {
                begin = helper::random_value<T>();
                end = helper::random_value<T>();
            }

            run_tween_test<T>("tween_from_to", easing_type, begin, end, object);
            run_tween_test<T>("tween_to", easing_type, begin, end, object);
            run_tween_test<T>("tween_by", easing_type, begin, end, {});
        });
    }
    test_scopes();
}

} //end of namespace tweeny
