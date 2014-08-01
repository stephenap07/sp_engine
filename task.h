#ifndef _SP_TASK_H_
#define _SP_TASK_H_

#include <SDL2/SDL.h>
#include <memory>
#include <list>

namespace sp {

class Task;
typedef std::shared_ptr<Task> TaskPtr;

class Task {
    friend class TaskManager;    

public:
    // Make this a non-copyable class
    // The weird "= delete" means we're not allowing this function to be used.
    Task() = default; 
    Task(const Task &) = delete;
    Task & operator=(const Task &) = delete;

public:
    Task(int type, unsigned int order=0); 
    virtual ~Task();

    bool IsDead() const { return is_dead; }
    int GetType() const { return task_type; }
    void SetType(const int type) { task_type = type; }
    bool IsActive() const { return is_active; }
    void SetActive(const bool active) { is_active = active; }
    bool IsPaused() const { return is_paused; }
    bool IsInitialized() const { return !initial_update; }
    TaskPtr const GetNext() const { return next_task; }
    void SetNext(TaskPtr next);

    virtual void OnUpdate(const Uint32 deltaMs);
    virtual void OnInitialize() {};
    virtual void Kill();
    virtual void TogglePause() {}

protected:
    int task_type;
    bool is_dead;
    bool is_active;
    bool is_paused;

    bool initial_update;
    std::shared_ptr<Task> next_task;

private:
    unsigned int task_flags;
};

inline void Task::OnUpdate(Uint32 deltaMs)
{
    if (!initial_update) {
        OnInitialize();
        initial_update = false;
    }
}

typedef std::list<TaskPtr> TaskList;
class TaskManager {
public:
    ~TaskManager();

    void Attach(TaskPtr task);
    bool HasProcesses();
    bool IsProcessActive(int type);
    void UpdateProcesses(const Uint32 deltaMs);

protected:
    TaskList task_list;

private:
    void Detach(TaskPtr task);
};

} // namespace sp

#endif
