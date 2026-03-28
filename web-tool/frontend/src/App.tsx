import { Routes, Route, Navigate } from 'react-router-dom'
import Layout from './components/Layout'
import Dashboard from './pages/Dashboard'
import TaskList from './pages/TaskList'
import TaskDetail from './pages/TaskDetail'
import CreateTask from './pages/CreateTask'
import Login from './pages/Login'

function App() {
  // TODO: Implement auth check
  const isAuthenticated = true

  if (!isAuthenticated) {
    return <Login />
  }

  return (
    <Routes>
      <Route path="/" element={<Layout />}>
        <Route index element={<Dashboard />} />
        <Route path="tasks" element={<TaskList />} />
        <Route path="tasks/create" element={<CreateTask />} />
        <Route path="tasks/:id" element={<TaskDetail />} />
        <Route path="*" element={<Navigate to="/" replace />} />
      </Route>
    </Routes>
  )
}

export default App