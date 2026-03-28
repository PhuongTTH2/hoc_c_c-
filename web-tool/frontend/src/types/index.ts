export interface Task {
  id: string
  task_key: string
  name: string
  description: string
  status: 'DRAFT' | 'ACTIVE' | 'IN_PROGRESS' | 'COMPLETED' | 'FAILED'
  priority: 'LOW' | 'MEDIUM' | 'HIGH' | 'CRITICAL'
  jira_id: string | null
  git_id: string | null
  sharepoint_id: string | null
  confluence_id: string | null
  created_by: string | null
  created_at: string
  updated_at: string
  executed_at: string | null
  completed_at: string | null
}

export interface JiraTask {
  id: string
  jira_ticket_key: string
  jira_ticket_url: string
  task_name: string
  task_description: string
  created_by: string
  assignee_name: string
  start_date: string | null
  due_date: string | null
  priority: string
  status: string
  project_key: string
}

export interface GitDetails {
  id: string
  branch_name: string
  branch_url: string
  repo_url: string
  repo_name: string
  base_branch: string
  provider: string
}

export interface SharePointDetails {
  id: string
  folder_name: string
  folder_path: string
  folder_url: string
  site_url: string
  template_name: string
  copied_file_url: string
}

export interface ConfluenceDetails {
  id: string
  page_id: string
  page_title: string
  page_url: string
  space_key: string
  page_version: number
}

export interface ExecutionLog {
  id: string
  task_id: string
  execution_number: number
  status: 'PENDING' | 'RUNNING' | 'SUCCESS' | 'PARTIAL_SUCCESS' | 'FAILED'
  started_at: string
  completed_at: string | null
  duration_ms: number | null
  jira_result: any
  git_result: any
  sharepoint_result: any
  confluence_result: any
  error_message: string | null
}

export interface StepLog {
  id: string
  execution_id: string
  step_order: number
  integration_type: 'JIRA' | 'GIT' | 'SHAREPOINT' | 'CONFLUENCE'
  endpoint: string
  method: string
  request_payload: any
  response_payload: any
  status_code: number
  status: 'SUCCESS' | 'FAILED' | 'RETRYING'
  error_message: string | null
  duration_ms: number
  retry_count: number
}