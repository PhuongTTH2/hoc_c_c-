import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query'
import { api, Task } from '../api/client'
import toast from 'react-hot-toast'

export const useTasks = () => {
  const queryClient = useQueryClient()

  const { data: tasks, isLoading, error } = useQuery({
    queryKey: ['tasks'],
    queryFn: () => api.getTasks().then(res => res.data.data),
  })

  const createTask = useMutation({
    mutationFn: (data: Partial<Task>) => api.createTask(data),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['tasks'] })
      toast.success('Task created successfully')
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to create task')
    },
  })

  const updateTask = useMutation({
    mutationFn: ({ id, data }: { id: string; data: Partial<Task> }) =>
      api.updateTask(id, data),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['tasks'] })
      toast.success('Task updated successfully')
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to update task')
    },
  })

  const deleteTask = useMutation({
    mutationFn: (id: string) => api.deleteTask(id),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['tasks'] })
      toast.success('Task deleted successfully')
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to delete task')
    },
  })

  const executeTask = useMutation({
    mutationFn: (id: string) => api.executeTask(id),
    onSuccess: () => {
      toast.success('Task execution started!')
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to execute task')
    },
  })

  return {
    tasks,
    isLoading,
    error,
    createTask,
    updateTask,
    deleteTask,
    executeTask,
  }
}