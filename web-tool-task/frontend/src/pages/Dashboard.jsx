import { useState, useEffect } from 'react'
import api from '../api/client'
import toast from 'react-hot-toast'
import KpiCard from '../components/KpiCard'
import StatusBadge from '../components/StatusBadge'
import Charts from '../components/Charts'

function Dashboard() {
  const [tasks, setTasks] = useState([])
  const [jiraTasks, setJiraTasks] = useState([])
  const [stats, setStats] = useState({ 
    total_tasks: 0, 
    completed: 0, 
    in_progress: 0, 
    draft: 0,
    completion_rate: 0 
  })
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    fetchData()
  }, [])

  const fetchData = async () => {
    setLoading(true)
    try {
      // Lấy tasks từ database
      const tasksRes = await api.get('/dashboard/tasks-list')
      setTasks(tasksRes.data.data || [])
      
      // Lấy Jira tasks
      const jiraRes = await api.get('/dashboard/jira-tasks')
      setJiraTasks(jiraRes.data.data || [])
      
      // Lấy stats
      const statsRes = await api.get('/dashboard/stats')
      setStats(statsRes.data)
      
    } catch (err) {
      console.error('Fetch error:', err)
      toast.error('Failed to load dashboard data')
    } finally {
      setLoading(false)
    }
  }

  const handleExecute = async (taskId) => {
    try {
      await api.post(`/tasks/${taskId}/execute`)
      toast.success('Task execution started!')
      fetchData()
    } catch (err) {
      toast.error('Failed to execute task')
    }
  }

  if (loading) {
    return (
      <div style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '400px' }}>
        <div style={{ textAlign: 'center' }}>
          <div style={{ fontSize: '48px', marginBottom: '16px' }}>⏳</div>
          <p>Loading dashboard...</p>
        </div>
      </div>
    )
  }

  return (
    <div>
      <h1 style={{ fontSize: '24px', fontWeight: 'bold', marginBottom: '24px' }}>Dashboard</h1>
      
      {/* KPI Cards */}
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '20px', marginBottom: '30px' }}>
        <KpiCard title="Total Tasks" value={stats.total_tasks} icon="📋" />
        <KpiCard title="Completed" value={stats.completed} icon="✅" />
        <KpiCard title="In Progress" value={stats.in_progress} icon="🔄" />
        <KpiCard title="Completion Rate" value={`${stats.completion_rate}%`} icon="📊" />
      </div>

      {/* Charts */}
      <Charts tasks={tasks} />

      {/* Jira Tasks Table */}
      <div style={{ background: 'white', borderRadius: '20px', border: '1px solid #e2e8f0', marginBottom: '30px', overflow: 'hidden' }}>
        <div style={{ padding: '16px 24px', borderBottom: '1px solid #eee', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <h2 style={{ fontSize: '18px' }}>📋 JIRA TASKS</h2>
          <button onClick={fetchData} style={{ background: 'none', border: '1px solid #ddd', padding: '6px 16px', borderRadius: '20px', cursor: 'pointer' }}>
            🔄 Sync Now
          </button>
        </div>
        <div style={{ overflowX: 'auto', padding: '0 24px 24px' }}>
          {jiraTasks.length === 0 ? (
            <div style={{ textAlign: 'center', padding: '48px', color: '#666' }}>
              <div style={{ fontSize: '48px', marginBottom: '16px' }}>📭</div>
              <p>No Jira tasks found</p>
              <p style={{ fontSize: '12px' }}>Upload a JSON file to create tasks</p>
            </div>
          ) : (
            <table style={{ width: '100%', borderCollapse: 'collapse' }}>
              <thead>
                <tr style={{ borderBottom: '1px solid #eee' }}>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Ticket</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Task Name</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Assignee</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Due Date</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Priority</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Status</th>
                </tr>
              </thead>
              <tbody>
                {jiraTasks.map(task => (
                  <tr key={task.id} style={{ borderBottom: '1px solid #f0f0f0' }}>
                    <td style={{ padding: '12px 8px' }}>
                      <a href={task.jira_ticket_url} target="_blank" rel="noopener noreferrer" style={{ color: '#0052CC', textDecoration: 'none', fontWeight: 'bold' }}>
                        {task.jira_ticket_key}
                      </a>
                    </td>
                    <td style={{ padding: '12px 8px' }}>{task.task_name}</td>
                    <td style={{ padding: '12px 8px' }}>{task.assignee_name || '-'}</td>
                    <td style={{ padding: '12px 8px', color: task.due_date && new Date(task.due_date) < new Date() ? '#dc2626' : '#666' }}>
                      {task.due_date || '-'}
                      {task.due_date && new Date(task.due_date) < new Date() && <span style={{ marginLeft: '8px', fontSize: '10px', color: '#dc2626' }}>⚠️</span>}
                    </td>
                    <td style={{ padding: '12px 8px' }}>
                      <span style={{ 
                        color: task.priority === 'High' ? '#dc2626' : 
                               task.priority === 'Medium' ? '#f59e0b' : '#10b981',
                        fontWeight: task.priority === 'High' ? 'bold' : 'normal'
                      }}>
                        {task.priority}
                      </span>
                    </td>
                    <td style={{ padding: '12px 8px' }}>
                      <StatusBadge status={task.status} />
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}
        </div>
      </div>

      {/* Uploaded Tasks List */}
      <div style={{ background: 'white', borderRadius: '20px', border: '1px solid #e2e8f0', marginBottom: '30px', overflow: 'hidden' }}>
        <div style={{ padding: '16px 24px', borderBottom: '1px solid #eee' }}>
          <h2 style={{ fontSize: '18px' }}>📋 UPLOADED TASKS</h2>
        </div>
        <div style={{ overflowX: 'auto', padding: '0 24px 24px' }}>
          {tasks.length === 0 ? (
            <div style={{ textAlign: 'center', padding: '48px', color: '#666' }}>
              <div style={{ fontSize: '48px', marginBottom: '16px' }}>📭</div>
              <p>No tasks uploaded yet</p>
              <p style={{ fontSize: '12px' }}>Go to <strong>Upload Task</strong> page to create tasks</p>
            </div>
          ) : (
            <table style={{ width: '100%', borderCollapse: 'collapse' }}>
              <thead>
                <tr style={{ borderBottom: '1px solid #eee' }}>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Task Key</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Task Name</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Priority</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Status</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Integrations</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px', color: '#666' }}>Action</th>
                </tr>
              </thead>
              <tbody>
                {tasks.map(task => (
                  <tr key={task.id} style={{ borderBottom: '1px solid #f0f0f0' }}>
                    <td style={{ padding: '12px 8px' }}><strong>{task.task_key}</strong></td>
                    <td style={{ padding: '12px 8px' }}>{task.name}</td>
                    <td style={{ padding: '12px 8px' }}>
                      <span style={{ 
                        color: task.priority === 'HIGH' ? '#dc2626' : 
                               task.priority === 'MEDIUM' ? '#f59e0b' : '#10b981'
                      }}>
                        {task.priority}
                      </span>
                    </td>
                    <td style={{ padding: '12px 8px' }}><StatusBadge status={task.status} /></td>
                    <td style={{ padding: '12px 8px' }}>
                      <div style={{ display: 'flex', gap: '4px', flexWrap: 'wrap' }}>
                        {task.has_jira && <span style={{ fontSize: '11px', background: '#e2e8f0', padding: '2px 6px', borderRadius: '10px' }}>Jira</span>}
                        {task.has_git && <span style={{ fontSize: '11px', background: '#e2e8f0', padding: '2px 6px', borderRadius: '10px' }}>Git</span>}
                        {task.has_sharepoint && <span style={{ fontSize: '11px', background: '#e2e8f0', padding: '2px 6px', borderRadius: '10px' }}>SPO</span>}
                        {task.has_confluence && <span style={{ fontSize: '11px', background: '#e2e8f0', padding: '2px 6px', borderRadius: '10px' }}>Conf</span>}
                        {!task.has_jira && !task.has_git && !task.has_sharepoint && !task.has_confluence && 
                          <span style={{ fontSize: '11px', color: '#666' }}>Pending</span>
                        }
                      </div>
                    </td>
                    <td style={{ padding: '12px 8px' }}>
                      <button 
                        onClick={() => handleExecute(task.id)} 
                        style={{ background: '#22c55e', color: 'white', border: 'none', padding: '4px 12px', borderRadius: '6px', cursor: 'pointer', fontSize: '12px' }}
                      >
                        Execute
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}
        </div>
      </div>

      {/* Overdue Tasks */}
      <div style={{ background: 'white', borderRadius: '20px', border: '1px solid #e2e8f0', overflow: 'hidden' }}>
        <div style={{ padding: '16px 24px', borderBottom: '1px solid #eee' }}>
          <h2 style={{ fontSize: '18px' }}>⚠️ OVERDUE TASKS</h2>
        </div>
        <div style={{ overflowX: 'auto', padding: '0 24px 24px' }}>
          {tasks.filter(t => t.status === 'OVERDUE').length === 0 ? (
            <div style={{ textAlign: 'center', padding: '48px', color: '#666' }}>
              <div style={{ fontSize: '48px', marginBottom: '16px' }}>✅</div>
              <p>No overdue tasks</p>
            </div>
          ) : (
            <table style={{ width: '100%', borderCollapse: 'collapse' }}>
              <thead>
                <tr>
                  <th style={{ textAlign: 'left', padding: '12px 8px' }}>Task Key</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px' }}>Task Name</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px' }}>Status</th>
                  <th style={{ textAlign: 'left', padding: '12px 8px' }}>Action</th>
                </tr>
              </thead>
              <tbody>
                {tasks.filter(t => t.status === 'OVERDUE').slice(0, 5).map(task => (
                  <tr key={task.id} style={{ background: '#fef2f2' }}>
                    <td style={{ padding: '12px 8px' }}><strong>{task.task_key}</strong></td>
                    <td style={{ padding: '12px 8px' }}>{task.name}</td>
                    <td style={{ padding: '12px 8px' }}><StatusBadge status="Overdue" /></td>
                    <td style={{ padding: '12px 8px' }}>
                      <button onClick={() => handleExecute(task.id)} style={{ background: '#dc2626', color: 'white', border: 'none', padding: '4px 12px', borderRadius: '6px', cursor: 'pointer' }}>
                        Execute Now
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}
        </div>
      </div>
    </div>
  )
}

export default Dashboard