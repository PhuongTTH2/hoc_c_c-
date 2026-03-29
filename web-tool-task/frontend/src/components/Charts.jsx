function Charts({ tasks }) {
  const statusCount = {
    'DRAFT': tasks.filter(t => t.status === 'DRAFT').length,
    'IN_PROGRESS': tasks.filter(t => t.status === 'IN_PROGRESS').length,
    'COMPLETED': tasks.filter(t => t.status === 'COMPLETED').length,
    'FAILED': tasks.filter(t => t.status === 'FAILED').length,
  }
  
  const total = tasks.length || 1
  const draftPercent = (statusCount['DRAFT'] / total) * 100
  const progressPercent = (statusCount['IN_PROGRESS'] / total) * 100
  const completedPercent = (statusCount['COMPLETED'] / total) * 100
  const failedPercent = (statusCount['FAILED'] / total) * 100

  // Priority counts from tasks
  const highCount = tasks.filter(t => t.priority === 'HIGH').length
  const mediumCount = tasks.filter(t => t.priority === 'MEDIUM').length
  const lowCount = tasks.filter(t => t.priority === 'LOW').length

  return (
    <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '24px', marginBottom: '30px' }}>
      {/* Pie Chart */}
      <div style={{ background: 'white', borderRadius: '20px', border: '1px solid #e2e8f0', padding: '20px' }}>
        <h3 style={{ fontSize: '14px', color: '#666', marginBottom: '20px' }}>🥧 Task Distribution by Status</h3>
        {total === 1 && tasks.length === 0 ? (
          <div style={{ textAlign: 'center', padding: '40px', color: '#666' }}>
            <p>No data yet. Upload a task to see chart.</p>
          </div>
        ) : (
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', gap: '40px', flexWrap: 'wrap' }}>
            <div style={{ position: 'relative', width: '180px', height: '180px', borderRadius: '50%', background: `conic-gradient(#f97316 0% ${draftPercent}%, #3b82f6 ${draftPercent}% ${draftPercent + progressPercent}%, #22c55e ${draftPercent + progressPercent}% ${draftPercent + progressPercent + completedPercent}%, #ef4444 ${draftPercent + progressPercent + completedPercent}% 100%)` }}>
              <div style={{ position: 'absolute', top: '50%', left: '50%', transform: 'translate(-50%, -50%)', width: '100px', height: '100px', background: 'white', borderRadius: '50%', display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', textAlign: 'center' }}>
                <div style={{ fontSize: '24px', fontWeight: 'bold' }}>{total}</div>
                <div style={{ fontSize: '10px', color: '#666' }}>Total Tasks</div>
              </div>
            </div>
            <div>
              <div><span style={{ display: 'inline-block', width: '12px', height: '12px', background: '#f97316', borderRadius: '3px', marginRight: '8px' }}></span> Draft ({statusCount['DRAFT']}) - {Math.round(draftPercent)}%</div>
              <div><span style={{ display: 'inline-block', width: '12px', height: '12px', background: '#3b82f6', borderRadius: '3px', marginRight: '8px' }}></span> In Progress ({statusCount['IN_PROGRESS']}) - {Math.round(progressPercent)}%</div>
              <div><span style={{ display: 'inline-block', width: '12px', height: '12px', background: '#22c55e', borderRadius: '3px', marginRight: '8px' }}></span> Completed ({statusCount['COMPLETED']}) - {Math.round(completedPercent)}%</div>
              <div><span style={{ display: 'inline-block', width: '12px', height: '12px', background: '#ef4444', borderRadius: '3px', marginRight: '8px' }}></span> Failed ({statusCount['FAILED']}) - {Math.round(failedPercent)}%</div>
            </div>
          </div>
        )}
      </div>

      {/* Bar Chart */}
      <div style={{ background: 'white', borderRadius: '20px', border: '1px solid #e2e8f0', padding: '20px' }}>
        <h3 style={{ fontSize: '14px', color: '#666', marginBottom: '20px' }}>📊 Tasks by Priority</h3>
        {tasks.length === 0 ? (
          <div style={{ textAlign: 'center', padding: '40px', color: '#666' }}>
            <p>No data yet. Upload a task to see chart.</p>
          </div>
        ) : (
          <div>
            <div style={{ marginBottom: '16px' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '12px', marginBottom: '6px' }}>
                <span>🔴 High Priority</span>
                <span>{highCount} tasks</span>
              </div>
              <div style={{ height: '8px', background: '#e2e8f0', borderRadius: '10px', overflow: 'hidden' }}>
                <div style={{ width: `${(highCount / total) * 100}%`, height: '100%', background: '#dc2626', borderRadius: '10px' }}></div>
              </div>
            </div>
            <div style={{ marginBottom: '16px' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '12px', marginBottom: '6px' }}>
                <span>🟡 Medium Priority</span>
                <span>{mediumCount} tasks</span>
              </div>
              <div style={{ height: '8px', background: '#e2e8f0', borderRadius: '10px', overflow: 'hidden' }}>
                <div style={{ width: `${(mediumCount / total) * 100}%`, height: '100%', background: '#eab308', borderRadius: '10px' }}></div>
              </div>
            </div>
            <div style={{ marginBottom: '16px' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '12px', marginBottom: '6px' }}>
                <span>🟢 Low Priority</span>
                <span>{lowCount} tasks</span>
              </div>
              <div style={{ height: '8px', background: '#e2e8f0', borderRadius: '10px', overflow: 'hidden' }}>
                <div style={{ width: `${(lowCount / total) * 100}%`, height: '100%', background: '#22c55e', borderRadius: '10px' }}></div>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}

export default Charts