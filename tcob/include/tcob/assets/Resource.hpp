// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <cassert>
#include <unordered_map>

namespace tcob {

template <typename T>
class ResourcePtr;

////////////////////////////////////////////////////////////

enum class ResourceState {
    Unloaded,
    Created,
    Loaded
};

////////////////////////////////////////////////////////////

class ResourceLoaderBase {
public:
    explicit ResourceLoaderBase(ResourceGroup& group);
    virtual ~ResourceLoaderBase() = default;

    virtual void resource_state(std::unordered_map<ResourceState, u32>& out) const = 0;

protected:
    virtual void on_loading() { }
    virtual void on_preparing() { }
    virtual void on_unloading() { }
    virtual void on_updating() { }

    auto group() -> ResourceGroup&;

private:
    ResourceGroup& _group;
};

////////////////////////////////////////////////////////////

template <typename T>
class ResourceLoader : public ResourceLoaderBase {
public:
    explicit ResourceLoader(ResourceGroup& group)
        : ResourceLoaderBase { group }
    {
    }

    auto get(const std::string& resname) const -> ResourcePtr<T>
    {
        if (!has(resname)) {
            return ResourcePtr<T> { nullptr };
        }
        return _resources.at(resname);
    }

    auto has(const std::string& resname) const -> bool
    {
        return _resources.contains(resname);
    }

    auto load(const std::string& resname) -> bool
    {
        if (!_resources.contains(resname)) {
            //TODO: log error
            return false;
        }

        return do_load(_resources[resname]);
    }

    void unload(const std::string& resname, bool greedy)
    {
        if (!_resources.contains(resname)) {
            //TODO: log error
            return;
        }

        do_unload(_resources[resname], greedy);

        _objects.erase(resname);
        _resources.erase(resname);
    }

    auto reload(const std::string& resname) -> bool
    {
        if (!_resources.contains(resname)) {
            //TODO: log error
            return false;
        }

        return do_reload(_resources[resname]);
    }

    virtual void register_wrapper(lua::Script& script) = 0;

    void resource_state(std::unordered_map<ResourceState, u32>& out) const override
    {
        for (auto& [_, res] : _resources) {
            out[res.get()->state()]++;
        }
    }

protected:
    virtual auto do_load(ResourcePtr<T>) -> bool { return true; }
    virtual void do_unload(ResourcePtr<T>, bool) {};
    virtual auto do_reload(ResourcePtr<T>) -> bool { return false; }

    template <typename R = T, typename... Args>
    auto get_or_create_resource(const std::string& resname, Args&&... args) -> ResourcePtr<T>
    {
        if (!_resources.contains(resname)) {
            auto obj { std::make_shared<R>(args...) };
            _objects[resname] = obj;
            _resources[resname] = { std::make_shared<Resource<T>>(obj, resname, this) };
        }

        return _resources[resname];
    }

    void set_resource_loaded(ResourcePtr<T> res)
    {
        res.get()->set_loaded();
    }

private:
    std::unordered_map<std::string, ResourcePtr<T>> _resources;
    std::unordered_map<std::string, std::shared_ptr<T>> _objects;
};

////////////////////////////////////////////////////////////

template <typename T>
class Resource {
public:
    Resource() = default;

    explicit Resource(std::weak_ptr<T> ptr)
        : _object { ptr }
    {
        _state = ResourceState::Loaded;
    }

    Resource(std::weak_ptr<T> ptr, const std::string& name, ResourceLoader<T>* loader)
        : _name { name }
        , _object { ptr }
        , _loader { loader }
    {
        _state = ResourceState::Created;
    }

    void unload(bool greedy = false)
    {
        if (_loader)
            _loader->unload(_name, greedy);

        assert(!valid());
        _state = ResourceState::Unloaded;
    }

    auto reload()
    {
        if (_loader)
            return _loader->reload(_name);

        return false;
    }

    auto name() const -> const std::string&
    {
        return _name;
    }

    auto valid() const -> bool
    {
        return _state != ResourceState::Unloaded && !_object.expired();
    }

    auto object() const -> T*
    {
        return object_ptr().get();
    }

    auto object_ptr() const -> std::shared_ptr<T>
    {
        return _object.lock();
    }

    auto state() const -> ResourceState
    {
        return _state;
    }

private:
    template <typename U>
    friend class ResourceLoader;

    void set_loaded()
    {
        _state = ResourceState::Loaded;
    }

    std::string _name;
    std::weak_ptr<T> _object;
    ResourceLoader<T>* _loader;
    ResourceState _state { ResourceState::Unloaded };
};

////////////////////////////////////////////////////////////

template <typename T>
class ResourcePtr {
public:
    ResourcePtr() = default;

    ResourcePtr(std::shared_ptr<Resource<T>> ptr)
        : _object { ptr }
    {
    }

    auto get() const -> Resource<T>*
    {
        return _object.get();
    }

    auto object() const -> T*
    {
        return _object->object();
    }

    auto operator->() const -> T*
    {
        return object();
    }

    auto operator*() const -> T&
    {
        return *object();
    }

    operator bool() const
    {
        return _object && _object->valid();
    }

private:
    std::shared_ptr<Resource<T>> _object;
};

}