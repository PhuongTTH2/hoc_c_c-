from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from app.core.database import get_db
from app.models.task import Task
from app.models.jira import JiraDetails
from datetime import datetime, timedelta

router = APIRouter()

@router.get("/stats")
def get_stats(db: Session = Depends(get_db)):
    total = db.query(Task).count()
    completed = db.query(Task).filter(Task.status == "COMPLETED").count()
    in_progress = db.query(Task).filter(Task.status == "IN_PROGRESS").count()
    draft = db.query(Task).filter(Task.status == "DRAFT").count()
    
    return {
        "total_tasks": total,
        "completed": completed,
        "in_progress": in_progress,
        "draft": draft,
        "completion_rate": round((completed / total * 100) if total > 0 else 0, 1)
    }

@router.get("/jira-tasks")
def get_jira_tasks(db: Session = Depends(get_db)):
    """Lấy danh sách Jira tasks từ database thật"""
    jira_tasks = db.query(JiraDetails).order_by(JiraDetails.created_at.desc()).limit(10).all()
    
    result = []
    for jira in jira_tasks:
        result.append({
            "id": str(jira.id),
            "jira_ticket_key": jira.jira_ticket_key,
            "jira_ticket_url": jira.jira_ticket_url,
            "task_name": jira.task_name,
            "task_description": jira.task_description,
            "created_by": jira.created_by,
            "assignee_name": jira.assignee_name,
            "start_date": jira.start_date.strftime("%Y-%m-%d") if jira.start_date else None,
            "due_date": jira.due_date.strftime("%Y-%m-%d") if jira.due_date else None,
            "priority": jira.priority,
            "status": jira.status,
            "project_key": jira.project_key
        })
    
    # Nếu chưa có dữ liệu Jira, trả về mock data để test
    if not result:
        result = [
            {
                "id": "1",
                "jira_ticket_key": "PROJ-123",
                "jira_ticket_url": "https://jira.atlassian.net/browse/PROJ-123",
                "task_name": "Implement Automation API",
                "task_description": "Build REST API for integrations",
                "created_by": "System",
                "assignee_name": "John Doe",
                "start_date": "2024-04-01",
                "due_date": "2024-04-15",
                "priority": "High",
                "status": "In Progress",
                "project_key": "PROJ"
            },
            {
                "id": "2",
                "jira_ticket_key": "PROJ-124",
                "jira_ticket_url": "https://jira.atlassian.net/browse/PROJ-124",
                "task_name": "Fix Login Bug",
                "task_description": "Fix OAuth2 authentication",
                "created_by": "System",
                "assignee_name": "Jane Smith",
                "start_date": "2024-04-05",
                "due_date": "2024-04-10",
                "priority": "Medium",
                "status": "Done",
                "project_key": "PROJ"
            }
        ]
    
    return {"data": result}

@router.get("/tasks-list")
def get_all_tasks(db: Session = Depends(get_db)):
    """Lấy danh sách tasks từ database"""
    tasks = db.query(Task).order_by(Task.created_at.desc()).all()
    
    result = []
    for task in tasks:
        result.append({
            "id": str(task.id),
            "task_key": task.task_key,
            "name": task.name,
            "description": task.description,
            "status": task.status,
            "priority": task.priority,
            "created_at": task.created_at.isoformat() if task.created_at else None,
            "has_jira": task.jira_id is not None,
            "has_git": task.git_id is not None,
            "has_sharepoint": task.sharepoint_id is not None,
            "has_confluence": task.confluence_id is not None
        })
    
    return {"data": result}