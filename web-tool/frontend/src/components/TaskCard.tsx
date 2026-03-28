import { Link } from 'react-router-dom'
import StatusBadge from './StatusBadge'
import { Task } from '../api/client'

interface TaskCardProps {
  task: Task
}

const TaskCard = ({ task }: TaskCardProps) => {
  const getPriorityColor = (priority: string) => {
    switch (priority?.toLowerCase()) {
      case 'high': return 'bg-red-100 text-red-800'
      case 'medium': return 'bg-yellow-100 text-yellow-800'
      case 'low': return 'bg-green-100 text-green-800'
      default: return 'bg-gray-100 text-gray-800'
    }
  }

  return (
    <Link to={`/tasks/${task.id}`}>
      <div className="bg-white rounded-lg shadow-md p-6 hover:shadow-lg transition-shadow">
        <div className="flex justify-between items-start mb-3">
          <h3 className="text-lg font-semibold text-gray-900">{task.name}</h3>
          <span className={`px-2 py-1 rounded-full text-xs font-medium ${getPriorityColor(task.priority)}`}>
            {task.priority || 'MEDIUM'}
          </span>
        </div>
        
        <p className="text-gray-600 text-sm mb-4 line-clamp-2">
          {task.description || 'No description'}
        </p>
        
        <div className="flex justify-between items-center">
          <StatusBadge status={task.status} />
          <span className="text-xs text-gray-400">
            {task.task_key}
          </span>
        </div>
      </div>
    </Link>
  )
}

export default TaskCard