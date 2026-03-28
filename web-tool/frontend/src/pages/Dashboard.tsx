import { useQuery } from '@tanstack/react-query'
import { api } from '../api/client'
import LoadingSpinner from '../components/LoadingSpinner'
import { Link } from 'react-router-dom'

const Dashboard = () => {
  const { data: tasks, isLoading: tasksLoading } = useQuery({
    queryKey: ['tasks'],
    queryFn: () => api.getTasks().then(res => res.data.data),
  })

  const { data: jiraTasks, isLoading: jiraLoading } = useQuery({
    queryKey: ['jira-tasks'],
    queryFn: () => api.getJiraTasks().then(res => res.data.data),
  })

  const { data: stats } = useQuery({
    queryKey: ['stats'],
    queryFn: () => api.getStats().then(res => res.data),
  })

  if (tasksLoading || jiraLoading) return <LoadingSpinner />

  const totalTasks = tasks?.length || 0
  const completedTasks = tasks?.filter(t => t.status === 'COMPLETED').length || 0
  const overdueTasks = jiraTasks?.filter(t => 
    t.due_date && new Date(t.due_date) < new Date() && t.status !== 'Done'
  ).length || 0

  return (
    <div className="space-y-6">
      {/* KPI Cards */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center">
            <div className="text-3xl mr-4">📋</div>
            <div>
              <p className="text-gray-500 text-sm">Total Tasks</p>
              <p className="text-2xl font-bold">{totalTasks}</p>
            </div>
          </div>
        </div>
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center">
            <div className="text-3xl mr-4">✅</div>
            <div>
              <p className="text-gray-500 text-sm">Completed</p>
              <p className="text-2xl font-bold">{completedTasks}</p>
            </div>
          </div>
        </div>
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center">
            <div className="text-3xl mr-4">⏰</div>
            <div>
              <p className="text-gray-500 text-sm">Overdue</p>
              <p className="text-2xl font-bold text-red-600">{overdueTasks}</p>
            </div>
          </div>
        </div>
        <div className="bg-white rounded-lg shadow p-6">
          <div className="flex items-center">
            <div className="text-3xl mr-4">📊</div>
            <div>
              <p className="text-gray-500 text-sm">Completion Rate</p>
              <p className="text-2xl font-bold">
                {totalTasks > 0 ? Math.round((completedTasks / totalTasks) * 100) : 0}%
              </p>
            </div>
          </div>
        </div>
      </div>

      {/* Recent Tasks */}
      <div className="bg-white rounded-lg shadow">
        <div className="px-6 py-4 border-b border-gray-200">
          <h2 className="text-lg font-medium text-gray-900">Recent Tasks</h2>
        </div>
        <div className="divide-y divide-gray-200">
          {tasks?.slice(0, 5).map((task) => (
            <div key={task.id} className="px-6 py-4 hover:bg-gray-50">
              <Link to={`/tasks/${task.id}`} className="block">
                <div className="flex justify-between items-center">
                  <div>
                    <p className="font-medium text-gray-900">{task.name}</p>
                    <p className="text-sm text-gray-500">{task.task_key}</p>
                  </div>
                  <span className={`px-2 py-1 rounded-full text-xs font-medium ${
                    task.status === 'COMPLETED' ? 'bg-green-100 text-green-800' :
                    task.status === 'FAILED' ? 'bg-red-100 text-red-800' :
                    'bg-blue-100 text-blue-800'
                  }`}>
                    {task.status}
                  </span>
                </div>
              </Link>
            </div>
          ))}
        </div>
      </div>

      {/* Jira Tasks */}
      <div className="bg-white rounded-lg shadow">
        <div className="px-6 py-4 border-b border-gray-200">
          <h2 className="text-lg font-medium text-gray-900">Jira Tasks</h2>
        </div>
        <div className="overflow-x-auto">
          <table className="min-w-full divide-y divide-gray-200">
            <thead className="bg-gray-50">
              <tr>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Key</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Task Name</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Assignee</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Due Date</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Priority</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Status</th>
              </tr>
            </thead>
            <tbody className="bg-white divide-y divide-gray-200">
              {jiraTasks?.slice(0, 10).map((task) => (
                <tr key={task.id} className="hover:bg-gray-50">
                  <td className="px-6 py-4 text-sm text-indigo-600">{task.jira_ticket_key}</td>
                  <td className="px-6 py-4 text-sm text-gray-900">{task.task_name}</td>
                  <td className="px-6 py-4 text-sm text-gray-500">{task.assignee_name}</td>
                  <td className={`px-6 py-4 text-sm ${
                    task.due_date && new Date(task.due_date) < new Date() && task.status !== 'Done'
                      ? 'text-red-600 font-medium'
                      : 'text-gray-500'
                  }`}>
                    {task.due_date || '-'}
                  </td>
                  <td className="px-6 py-4">
                    <span className={`px-2 py-1 rounded-full text-xs font-medium ${
                      task.priority === 'High' ? 'bg-red-100 text-red-800' :
                      task.priority === 'Medium' ? 'bg-yellow-100 text-yellow-800' :
                      'bg-green-100 text-green-800'
                    }`}>
                      {task.priority}
                    </span>
                  </td>
                  <td className="px-6 py-4 text-sm text-gray-500">{task.status}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  )
}

export default Dashboard