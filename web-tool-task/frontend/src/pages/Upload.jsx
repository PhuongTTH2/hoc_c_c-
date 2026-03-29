import { useState } from 'react'
import api from '../api/client'
import toast from 'react-hot-toast'

function Upload() {
  const [file, setFile] = useState(null)
  const [preview, setPreview] = useState(null)
  const [uploading, setUploading] = useState(false)
  const [result, setResult] = useState(null)

  const handleFileChange = (e) => {
    const selected = e.target.files[0]
    if (selected && selected.type === 'application/json') {
      setFile(selected)
      const reader = new FileReader()
      reader.onload = (event) => {
        try {
          const json = JSON.parse(event.target.result)
          setPreview(JSON.stringify(json, null, 2))
        } catch {
          setPreview('Invalid JSON format')
        }
      }
      reader.readAsText(selected)
      setResult(null)
    } else {
      toast.error('Please upload a JSON file')
    }
  }

  const handleUpload = async () => {
    if (!file) {
      toast.error('Please select a file first')
      return
    }

    setUploading(true)
    const formData = new FormData()
    formData.append('file', file)

    try {
      const response = await api.post('/tasks/upload/bulk', formData, {
        headers: { 'Content-Type': 'multipart/form-data' }
      })
      
      setResult(response.data.data)
      
      if (response.data.data.success > 0) {
        toast.success(`✅ ${response.data.data.success} tasks created successfully!`)
      }
      if (response.data.data.failed > 0) {
        toast.warning(`⚠️ ${response.data.data.failed} tasks failed`)
      }
      
      // Clear file after successful upload
      setFile(null)
      setPreview(null)
      document.getElementById('fileInput').value = ''
      
    } catch (error) {
      console.error('Upload error:', error)
      toast.error(error.response?.data?.detail || 'Upload failed')
    } finally {
      setUploading(false)
    }
  }

  const downloadSample = () => {
    const sample = {
      "tasks": [
        {
          "task_name": "Implement Automation API",
          "description": "Build REST API endpoints for Jira, Git, SharePoint, Confluence integration",
          "priority": "HIGH",
          "jira_config": {
            "project_key": "PROJ",
            "assignee": "john.doe@company.com",
            "priority": "High",
            "start_date": "2024-04-01",
            "due_date": "2024-04-15"
          },
          "git_config": {
            "repo_url": "https://github.com/company/web-tool",
            "branch_prefix": "feature"
          }
        },
        {
          "task_name": "Fix Login Bug",
          "description": "Fix OAuth2 authentication issue",
          "priority": "HIGH",
          "jira_config": {
            "project_key": "PROJ",
            "assignee": "jane.smith@company.com",
            "priority": "High"
          }
        }
      ]
    }
    const blob = new Blob([JSON.stringify(sample, null, 2)], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = 'bulk_tasks_sample.json'
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
    URL.revokeObjectURL(url)
    toast.success('Sample JSON downloaded!')
  }

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '24px' }}>
        <h1 style={{ fontSize: '24px', fontWeight: 'bold' }}>📁 Bulk Upload Tasks</h1>
        <button onClick={downloadSample} style={{ background: '#3b82f6', color: 'white', border: 'none', padding: '10px 20px', borderRadius: '40px', cursor: 'pointer', display: 'flex', alignItems: 'center', gap: '8px' }}>
          📥 Download Sample JSON
        </button>
      </div>
      
      <div style={{ background: 'white', borderRadius: '24px', border: '1px solid #e2e8f0', padding: '32px' }}>
        <div style={{ border: '2px dashed #cbd5e1', borderRadius: '16px', padding: '48px', textAlign: 'center', cursor: 'pointer', background: '#fafbfc' }} onClick={() => document.getElementById('fileInput').click()}>
          <div style={{ fontSize: '48px', marginBottom: '16px' }}>📦</div>
          <h3 style={{ fontSize: '18px', marginBottom: '8px' }}>Upload JSON file with multiple tasks</h3>
          <p style={{ color: '#666', fontSize: '14px', marginBottom: '16px' }}>File format: {"{ \"tasks\": [...] }"} or single task</p>
          <input id="fileInput" type="file" accept=".json" onChange={handleFileChange} style={{ display: 'none' }} />
          <button style={{ background: '#22c55e', color: 'white', border: 'none', padding: '10px 24px', borderRadius: '40px', cursor: 'pointer' }}>Choose File</button>
        </div>
        
        {file && (
          <div style={{ marginTop: '24px', background: '#f1f5f9', borderRadius: '12px', padding: '16px' }}>
            <div style={{ display: 'flex', alignItems: 'center', gap: '12px', marginBottom: '12px' }}>
              <span>📄</span>
              <span><strong>{file.name}</strong></span>
              <span style={{ color: '#666', fontSize: '12px' }}>{(file.size / 1024).toFixed(2)} KB</span>
            </div>
            <pre style={{ background: '#1e293b', color: '#e2e8f0', padding: '16px', borderRadius: '8px', fontSize: '12px', overflow: 'auto', maxHeight: '300px' }}>{preview}</pre>
            <button 
              onClick={handleUpload} 
              disabled={uploading}
              style={{ marginTop: '16px', background: uploading ? '#94a3b8' : '#22c55e', color: 'white', border: 'none', padding: '10px 24px', borderRadius: '40px', cursor: uploading ? 'not-allowed' : 'pointer', width: '100%' }}
            >
              {uploading ? '⏳ Processing...' : '🚀 Upload All Tasks'}
            </button>
          </div>
        )}

        {result && (
          <div style={{ marginTop: '24px', background: result.failed > 0 ? '#fef3c7' : '#dcfce7', borderRadius: '12px', padding: '16px', border: `1px solid ${result.failed > 0 ? '#f59e0b' : '#22c55e'}` }}>
            <h4 style={{ color: result.failed > 0 ? '#d97706' : '#16a34a', marginBottom: '12px' }}>
              {result.failed > 0 ? '⚠️ Partial Success' : '✅ All Tasks Created!'}
            </h4>
            <p><strong>Total:</strong> {result.total} | <strong>Success:</strong> {result.success} | <strong>Failed:</strong> {result.failed}</p>
            
            {result.results && result.results.length > 0 && (
              <details style={{ marginTop: '12px' }}>
                <summary style={{ cursor: 'pointer', fontSize: '12px', color: '#666' }}>View Details</summary>
                <div style={{ marginTop: '8px', maxHeight: '200px', overflow: 'auto' }}>
                  {result.results.map((r, idx) => (
                    <div key={idx} style={{ fontSize: '11px', padding: '4px 0', borderBottom: '1px solid #e2e8f0', color: r.status === 'success' ? '#16a34a' : '#dc2626' }}>
                      {r.status === 'success' ? '✅' : '❌'} {r.task_name || r.task_key} - {r.status}
                      {r.error && <span style={{ color: '#dc2626', fontSize: '10px', marginLeft: '8px' }}>({r.error})</span>}
                    </div>
                  ))}
                </div>
              </details>
            )}
          </div>
        )}
      </div>
      
      <div style={{ marginTop: '24px', background: 'white', borderRadius: '20px', border: '1px solid #e2e8f0', padding: '20px' }}>
        <h3 style={{ marginBottom: '12px' }}>📋 Bulk JSON Format Example</h3>
        <pre style={{ background: '#1e293b', color: '#e2e8f0', padding: '16px', borderRadius: '12px', fontSize: '12px', overflow: 'auto' }}>
{`{
  "tasks": [
    {
      "task_name": "Task 1",
      "description": "Description for task 1",
      "priority": "HIGH",
      "jira_config": { ... },
      "git_config": { ... }
    },
    {
      "task_name": "Task 2",
      "description": "Description for task 2",
      "priority": "MEDIUM",
      "jira_config": { ... }
    }
  ]
}`}
        </pre>
        <p style={{ fontSize: '12px', color: '#666', marginTop: '12px' }}>
          💡 <strong>Tip:</strong> You can upload a single task (without "tasks" wrapper) or multiple tasks (with "tasks" array)
        </p>
      </div>
    </div>
  )
}

export default Upload