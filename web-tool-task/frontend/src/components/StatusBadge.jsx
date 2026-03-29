function StatusBadge({ status }) {
  const getStyles = () => {
    switch (status?.toLowerCase()) {
      case 'to do': return { background: '#fef3c7', color: '#d97706' }
      case 'in progress': return { background: '#dbeafe', color: '#2563eb' }
      case 'done': return { background: '#dcfce7', color: '#16a34a' }
      case 'overdue': return { background: '#fee2e2', color: '#dc2626' }
      default: return { background: '#f1f5f9', color: '#475569' }
    }
  }
  const styles = getStyles()
  return <span style={{ display: 'inline-block', padding: '4px 10px', borderRadius: '20px', fontSize: '11px', fontWeight: '500', ...styles }}>{status || 'Unknown'}</span>
}

export default StatusBadge