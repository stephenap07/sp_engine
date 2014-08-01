#include "task.h"

namespace sp {

//=============================================================================

void Task::SetNext(TaskPtr task)
{
    next_task = task;
}

//=============================================================================

void TaskManager::Attach(TaskPtr task)
{
    task_list.push_back(task);
}

//=============================================================================

void TaskManager::Detach(TaskPtr task)
{
    task_list.remove(task);
}

//=============================================================================

void TaskManager::UpdateProcesses(Uint32 deltaMs)
{
    TaskPtr next;
    for (auto current : task_list) {
        if (current->IsDead()) {
            next = current->GetNext();
            if (next) {
                current->SetNext(TaskPtr(nullptr));
                Attach(next);
            }
            Detach(current);
        } else if (current->IsActive() && !current->IsPaused()) {
            current->OnUpdate(deltaMs);
        }
    }
}

} // namespace sp
