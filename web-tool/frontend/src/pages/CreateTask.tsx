import { useNavigate } from 'react-router-dom'
import { useMutation } from '@tanstack/react-query'
import toast from 'react-hot-toast'
import { api } from '../api/client'
import TaskForm from '../components/TaskForm'

const CreateTask = () => {
  const navigate = useNavigate()

  const createTask = useMutation({
    mutationFn: (data: any) => api.createTask(data),
    onSuccess: (response) => {
      toast.success('Task created successfully!')
      navigate(`/tasks/${response.data.data.id}`)
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to create task')
    },
  })

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold text-gray-900">Create New Task</h1>
      <TaskForm onSubmit={createTask.mutate} isLoading={createTask.isPending} />
    </div>
  )
}

export default CreateTask