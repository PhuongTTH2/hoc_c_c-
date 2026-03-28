import axios from 'axios'

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:8000'

const apiClient = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
})

// Request interceptor - add auth token
apiClient.interceptors.request.use(
  (config) => {
    const token = localStorage.getItem('token')
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => Promise.reject(error)
)

// Response interceptor - handle errors
apiClient.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error.response?.status === 401) {
      localStorage.removeItem('token')
      window.location.href = '/login'
    }
    return Promise.reject(error)
  }
)

// API Types
export interface Task {
  id: string
  task_key: string
  name: string
  description: string
  status: string
  priority: string
  jira_id: string | null
  git_id: string | null
  sharepoint_id: string | null
  confluence_id: string | null
  created_at: string
  updated_at: string
}

export interface JiraTask {
  id: string
  jira_ticket_key: string
  task_name: string
  created_by: string
  assignee_name: string
  start_date: string
  due_date: string
  priority: string
  status: string
}

export interface ExecutionLog {
  id: string
  task_id: string
  status: string
  started_at: string
  completed_at: string
  duration_ms: number
}

// API Functions
export const api = {
  // Tasks
  getTasks: () => apiClient.get<{ data: Task[] }>('/api/tasks'),
  getTask: (id: string) => apiClient.get<{ data: Task }>(`/api/tasks/${id}`),
  createTask: (data: Partial<Task>) => apiClient.post('/api/tasks', data),
  updateTask: (id: string, data: Partial<Task>) => apiClient.put(`/api/tasks/${id}`, data),
  deleteTask: (id: string) => apiClient.delete(`/api/tasks/${id}`),
  executeTask: (id: string) => apiClient.post(`/api/tasks/${id}/execute`),
  
  // Dashboard
  getJiraTasks: () => apiClient.get<{ data: JiraTask[] }>('/api/dashboard/jira-tasks'),
  getStats: () => apiClient.get('/api/dashboard/stats'),
  
  // Executions
  getExecutions: (taskId: string) => apiClient.get<{ data: ExecutionLog[] }>(`/api/tasks/${taskId}/executions`),
  getExecutionLogs: (executionId: string) => apiClient.get(`/api/executions/${executionId}/logs`),
  
  // Auth
  login: (email: string, password: string) => apiClient.post('/api/auth/login', { email, password }),
  register: (data: any) => apiClient.post('/api/auth/register', data),
}

export default apiClient