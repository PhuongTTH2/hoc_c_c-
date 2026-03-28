import { useQuery } from '@tanstack/react-query'
import { Link } from 'react-router-dom'
import { api } from '../api/client'
import TaskCard from '../components/TaskCard'
import LoadingSpinner from '../components/LoadingSpinner'

const TaskList = () => {
  const { data, isLoading, error } = useQuery({
    queryKey: ['tasks'],
    queryFn: () => api.getTasks().then(res => res.data.data),
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <div className="text-red-600">Error loading tasks: {error.message}</div>

  return (
    <div className="space-y-6">
      <div className="flex justify-between items-center">
        <h1 className="text-2xl font-bold text-gray-900">Tasks</h1>
        <Link
          to="/tasks/create"
          className="bg-indigo-600 text-white px-4 py-2 rounded-md hover:bg-indigo-700"
        >
          + Create Task
        </Link>
      </div>

      {data?.length === 0 ? (
        <div className="bg-white rounded-lg shadow p-12 text-center">
          <div className="text-6xl mb-4">📭</div>
          <h3 className="text-lg font-medium text-gray-900 mb-2">No tasks yet</h3>
          <p className="text-gray-500 mb-4">Create your first task to get started</p>
          <Link
            to="/tasks/create"
            className="bg-indigo-600 text-white px-4 py-2 rounded-md hover:bg-indigo-700"
          >
            Create Task
          </Link>
        </div>
      ) : (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {data?.map((task) => (
            <TaskCard key={task.id} task={task} />
          ))}
        </div>
      )}
    </div>
  )
}

export default TaskList