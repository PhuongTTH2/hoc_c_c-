import { useParams, useNavigate } from 'react-router-dom'
import { useQuery, useMutation } from '@tanstack/react-query'
import toast from 'react-hot-toast'
import { api } from '../api/client'
import LoadingSpinner from '../components/LoadingSpinner'
import StatusBadge from '../components/StatusBadge'

const TaskDetail = () => {
  const { id } = useParams<{ id: string }>()
  const navigate = useNavigate()

  const { data, isLoading, refetch } = useQuery({
    queryKey: ['task', id],
    queryFn: () => api.getTask(id!).then(res => res.data.data),
    enabled: !!id,
  })

  const executeMutation = useMutation({
    mutationFn: () => api.executeTask(id!),
    onSuccess: () => {
      toast.success('Task execution started! You will be notified when complete.')
      refetch()
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to execute task')
    },
  })

  const deleteMutation = useMutation({
    mutationFn: () => api.deleteTask(id!),
    onSuccess: () => {
      toast.success('Task deleted successfully')
      navigate('/tasks')
    },
    onError: (error: any) => {
      toast.error(error.response?.data?.message || 'Failed to delete task')
    },
  })

  if (isLoading) return <LoadingSpinner />
  if (!data) return <div className="text-center py-10">Task not found</div>

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex justify-between items-start">
        <div>
          <div className="flex items-center gap-3 mb-2">
            <h1 className="text-2xl font-bold text-gray-900">{data.name}</h1>
            <StatusBadge status={data.status} />
          </div>
          <p className="text-gray-500">{data.task_key}</p>
        </div>
        <div className="flex gap-3">
          <button
            onClick={() => executeMutation.mutate()}
            disabled={executeMutation.isPending}
            className="bg-green-600 text-white px-4 py-2 rounded-md hover:bg-green-700 disabled:opacity-50"
          >
            {executeMutation.isPending ? 'Executing...' : '▶ Execute'}
          </button>
          <button
            onClick={() => deleteMutation.mutate()}
            disabled={deleteMutation.isPending}
            className="bg-red-600 text-white px-4 py-2 rounded-md hover:bg-red-700 disabled:opacity-50"
          >
            {deleteMutation.isPending ? 'Deleting...' : '🗑 Delete'}
          </button>
        </div>
      </div>

      {/* Description */}
      <div className="bg-white rounded-lg shadow p-6">
        <h2 className="text-lg font-medium text-gray-900 mb-2">Description</h2>
        <p className="text-gray-700">{data.description || 'No description'}</p>
      </div>

      {/* Integration Links */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {/* Jira */}
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center gap-2 mb-3">
            <span className="text-2xl">📋</span>
            <h2 className="text-lg font-medium text-gray-900">Jira</h2>
          </div>
          {data.jira_id ? (
            <div>
              <p className="text-sm text-gray-500">Ticket created</p>
              <a href="#" className="text-indigo-600 hover:underline">View in Jira</a>
            </div>
          ) : (
            <p className="text-gray-500">Not yet created</p>
          )}
        </div>

        {/* Git */}
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center gap-2 mb-3">
            <span className="text-2xl">🔀</span>
            <h2 className="text-lg font-medium text-gray-900">Git</h2>
          </div>
          {data.git_id ? (
            <div>
              <p className="text-sm text-gray-500">Branch created</p>
              <a href="#" className="text-indigo-600 hover:underline">View in Git</a>
            </div>
          ) : (
            <p className="text-gray-500">Not yet created</p>
          )}
        </div>

        {/* SharePoint */}
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center gap-2 mb-3">
            <span className="text-2xl">📁</span>
            <h2 className="text-lg font-medium text-gray-900">SharePoint</h2>
          </div>
          {data.sharepoint_id ? (
            <div>
              <p className="text-sm text-gray-500">Folder created</p>
              <a href="#" className="text-indigo-600 hover:underline">View in SharePoint</a>
            </div>
          ) : (
            <p className="text-gray-500">Not yet created</p>
          )}
        </div>

        {/* Confluence */}
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center gap-2 mb-3">
            <span className="text-2xl">📄</span>
            <h2 className="text-lg font-medium text-gray-900">Confluence</h2>
          </div>
          {data.confluence_id ? (
            <div>
              <p className="text-sm text-gray-500">Page created</p>
              <a href="#" className="text-indigo-600 hover:underline">View in Confluence</a>
            </div>
          ) : (
            <p className="text-gray-500">Not yet created</p>
          )}
        </div>
      </div>

      {/* Metadata */}
      <div className="bg-gray-50 rounded-lg p-4 text-sm text-gray-500">
        <p>Created: {new Date(data.created_at).toLocaleString()}</p>
        <p>Last updated: {new Date(data.updated_at).toLocaleString()}</p>
      </div>
    </div>
  )
}

export default TaskDetail