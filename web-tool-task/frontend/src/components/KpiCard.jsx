function KpiCard({ title, value, icon, trend, trendUp }) {
  return (
    <div style={{ background: 'white', borderRadius: '20px', padding: '20px', border: '1px solid #e2e8f0', transition: 'all 0.3s' }}>
      <div style={{ color: '#666', fontSize: '14px', marginBottom: '8px', display: 'flex', alignItems: 'center', gap: '8px' }}>
        <span>{icon}</span> {title}
      </div>
      <div style={{ fontSize: '32px', fontWeight: 'bold', color: '#1a1a2e' }}>{value}</div>
      {trend && <div style={{ fontSize: '12px', marginTop: '8px', color: trendUp ? '#22c55e' : '#666' }}>{trend}</div>}
    </div>
  )
}

export default KpiCard