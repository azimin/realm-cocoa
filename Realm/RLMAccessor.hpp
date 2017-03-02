////////////////////////////////////////////////////////////////////////////
//
// Copyright 2017 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#import "RLMAccessor.h"

#import "object_accessor.hpp"

#import "RLMUtil.hpp"

@class RLMRealm;
class RLMClassInfo;
class RLMObservationInfo;

class RLMAccessorContext {
public:
    RLMAccessorContext(RLMObjectBase *parentObject);
    RLMAccessorContext(RLMRealm *realm, RLMClassInfo& info, bool is_create);

    id defaultValue(NSString *key);
    id value(id obj, size_t propIndex);

    id wrap(realm::List);
    id wrap(realm::Results);
    id wrap(realm::Object);

    size_t addObject(id value, std::string const& object_type, bool is_update);

    RLMProperty *currentProperty;

    void will_change(realm::Row const&, realm::Property const&);
    void did_change(realm::Row const&, realm::Property const&);

private:
    RLMRealm *_realm;
    RLMClassInfo& _info;
    bool _is_create;
    RLMObjectBase *_parentObject;
    NSDictionary *_defaultValues;

    RLMObservationInfo *_observationInfo = nullptr;
    NSString *_kvoPropertyName = nil;

    id doGetValue(id obj, size_t propIndex, __unsafe_unretained RLMProperty *const prop);
};

namespace realm {
template<>
class NativeAccessor<id, RLMAccessorContext*> {
public:
    static id value_for_property(RLMAccessorContext* c, id dict, std::string const&, size_t prop_index);

    static bool dict_has_value_for_key(RLMAccessorContext*, id dict, const std::string &prop_name);
    static id dict_value_for_key(RLMAccessorContext*, id dict, const std::string &prop_name);

    static size_t list_size(RLMAccessorContext*, id v);
    static id list_value_at_index(RLMAccessorContext*, id v, size_t index);

    template<typename Func>
    static void list_enumerate(RLMAccessorContext*, id v, Func&& func) {
        for (id value in v) {
            func(value);
        }
    }

    static bool has_default_value_for_property(RLMAccessorContext* c, Realm*, ObjectSchema const&,
                                               std::string const& prop);
    static id default_value_for_property(RLMAccessorContext* c, Realm*, ObjectSchema const&,
                                         std::string const& prop);

    static Timestamp to_timestamp(RLMAccessorContext*, id v) { return RLMTimestampForNSDate(v); }
    static bool to_bool(RLMAccessorContext*, id v) { return [v boolValue]; }
    static double to_double(RLMAccessorContext*, id v) { return [v doubleValue]; }
    static float to_float(RLMAccessorContext*, id v) { return [v floatValue]; }
    static long long to_long(RLMAccessorContext*, id v) { return [v longLongValue]; }
    static BinaryData to_binary(RLMAccessorContext*, id v) { return RLMBinaryDataForNSData(v); }
    static StringData to_string(RLMAccessorContext*, id v) { return RLMStringDataWithNSString(v); }
    static Mixed to_mixed(RLMAccessorContext*, id) { throw std::logic_error("'Any' type is unsupported"); }

    static id from_binary(RLMAccessorContext*, BinaryData v) { return RLMBinaryDataToNSData(v); }
    static id from_bool(RLMAccessorContext*, bool v) { return @(v); }
    static id from_double(RLMAccessorContext*, double v) { return @(v); }
    static id from_float(RLMAccessorContext*, float v) { return @(v); }
    static id from_long(RLMAccessorContext*, long long v) { return @(v); }
    static id from_string(RLMAccessorContext*, StringData v) { return @(v.data()); }
    static id from_timestamp(RLMAccessorContext*, Timestamp v) { return RLMTimestampToNSDate(v); }
    static id from_list(RLMAccessorContext* c, List v) { return c->wrap(std::move(v)); }
    static id from_results(RLMAccessorContext* c, Results v) { return c->wrap(std::move(v)); }
    static id from_object(RLMAccessorContext *c, Object v) { return c->wrap(v); }

    static bool is_null(RLMAccessorContext*, id v) { return v == NSNull.null; }
    static id null_value(RLMAccessorContext*) { return NSNull.null; }
    static bool allow_missing(RLMAccessorContext*, id v) { return [v isKindOfClass:[NSArray class]]; }

    static size_t to_existing_object_index(RLMAccessorContext*, SharedRealm, id &);
    static size_t to_object_index(RLMAccessorContext* c, SharedRealm realm, id value, std::string const& object_type, bool update);

    static void will_change(RLMAccessorContext* c, realm::Object& obj, realm::Property const& prop) { c->will_change(obj.row(), prop); }
    static void did_change(RLMAccessorContext* c, realm::Object& obj, realm::Property const& prop) { c->did_change(obj.row(), prop); }
};
}
