from fastapi import APIRouter, Depends, HTTPException, UploadFile, File
from sqlalchemy.orm import Session
from pydantic import BaseModel
from datetime import datetime
from app.core.database import get_db
from app.models.task import Task
from app.models.jira import JiraDetails
from app.models.git import GitDetails
from app.models.sharepoint import SharePointDetails
from app.models.confluence import ConfluenceDetails
import uuid
import json
import logging

router = APIRouter()
logger = logging.getLogger(__name__)

class TaskCreate(BaseModel):
    name: str
    description: str = ""
    priority: str = "MEDIUM"

class TaskUpdate(BaseModel):
    name: str = None
    description: str = None
    status: str = None
    priority: str = None

def generate_task_key():
    return f"TASK-{uuid.uuid4().hex[:8].upper()}"

def create_single_task(data: dict, db: Session):
    """Tạo một task từ dữ liệu JSON"""
    try:
        # 1. Create main Task
        task = Task(
            task_key=generate_task_key(),
            name=data.get("task_name", "Untitled Task"),
            description=data.get("description", ""),
            priority=data.get("priority", "MEDIUM"),
            status="DRAFT"
        )
        db.add(task)
        db.flush()
        
        # 2. Create Jira Details
        jira_config = data.get("jira_config")
        if jira_config:
            jira_detail = JiraDetails(
                jira_ticket_key=jira_config.get("project_key", "TEMP") + "-" + str(uuid.uuid4().hex[:6]).upper(),
                jira_ticket_url=f"https://jira.atlassian.net/browse/{jira_config.get('project_key', 'TEMP')}",
                task_name=data.get("task_name", ""),
                task_description=data.get("description", ""),
                created_by=jira_config.get("assignee", "System"),
                assignee_name=jira_config.get("assignee", ""),
                start_date=jira_config.get("start_date"),
                due_date=jira_config.get("due_date"),
                priority=jira_config.get("priority", "Medium"),
                issue_type=jira_config.get("issue_type", "Task"),
                project_key=jira_config.get("project_key", ""),
                status="To Do",
                labels=jira_config.get("labels", [])
            )
            db.add(jira_detail)
            db.flush()
            task.jira_id = jira_detail.id
        
        # 3. Create Git Details
        git_config = data.get("git_config")
        if git_config:
            git_detail = GitDetails(
                branch_name=git_config.get("branch_name", f"feature/{task.task_key.lower()}"),
                branch_url=git_config.get("repo_url", ""),
                repo_url=git_config.get("repo_url", ""),
                repo_name=git_config.get("repo_url", "").split("/")[-1] if git_config.get("repo_url") else "",
                base_branch=git_config.get("base_branch", "main"),
                provider="GITHUB"
            )
            db.add(git_detail)
            db.flush()
            task.git_id = git_detail.id
        
        # 4. Create SharePoint Details
        sharepoint_config = data.get("sharepoint_config")
        if sharepoint_config:
            sharepoint_detail = SharePointDetails(
                folder_name=sharepoint_config.get("folder_name", task.task_key),
                folder_path=sharepoint_config.get("folder_path", f"/Projects/{task.task_key}"),
                folder_url=sharepoint_config.get("site_url", ""),
                site_url=sharepoint_config.get("site_url", ""),
                template_name=sharepoint_config.get("template_name", ""),
                copied_file_url=sharepoint_config.get("site_url", "")
            )
            db.add(sharepoint_detail)
            db.flush()
            task.sharepoint_id = sharepoint_detail.id
        
        # 5. Create Confluence Details
        confluence_config = data.get("confluence_config")
        if confluence_config:
            confluence_detail = ConfluenceDetails(
                page_id=str(uuid.uuid4().int)[:10],
                page_title=confluence_config.get("page_title", f"{task.task_key} Documentation"),
                page_url="",
                space_key=confluence_config.get("space_key", "DEV"),
                content=confluence_config.get("page_template", "# Documentation"),
                labels=confluence_config.get("labels", [])
            )
            db.add(confluence_detail)
            db.flush()
            task.confluence_id = confluence_detail.id
        
        return task
        
    except Exception as e:
        logger.error(f"Error creating task: {e}")
        raise e


@router.get("/")
def get_tasks(db: Session = Depends(get_db)):
    tasks = db.query(Task).order_by(Task.created_at.desc()).all()
    return {"data": tasks}


@router.get("/{task_id}")
def get_task(task_id: str, db: Session = Depends(get_db)):
    task = db.query(Task).filter(Task.id == task_id).first()
    if not task:
        raise HTTPException(status_code=404, detail="Task not found")
    return {"data": task}


@router.post("/")
def create_task(data: TaskCreate, db: Session = Depends(get_db)):
    task = Task(
        task_key=generate_task_key(),
        name=data.name,
        description=data.description,
        priority=data.priority
    )
    db.add(task)
    db.commit()
    db.refresh(task)
    return {"data": task}


@router.put("/{task_id}")
def update_task(task_id: str, data: TaskUpdate, db: Session = Depends(get_db)):
    task = db.query(Task).filter(Task.id == task_id).first()
    if not task:
        raise HTTPException(status_code=404, detail="Task not found")
    
    for key, value in data.dict(exclude_unset=True).items():
        setattr(task, key, value)
    
    db.commit()
    db.refresh(task)
    return {"data": task}


@router.delete("/{task_id}")
def delete_task(task_id: str, db: Session = Depends(get_db)):
    task = db.query(Task).filter(Task.id == task_id).first()
    if not task:
        raise HTTPException(status_code=404, detail="Task not found")
    
    db.delete(task)
    db.commit()
    return {"message": "Task deleted"}


@router.post("/{task_id}/execute")
def execute_task(task_id: str, db: Session = Depends(get_db)):
    task = db.query(Task).filter(Task.id == task_id).first()
    if not task:
        raise HTTPException(status_code=404, detail="Task not found")
    
    task.status = "IN_PROGRESS"
    task.executed_at = datetime.utcnow()
    db.commit()
    
    return {
        "execution_id": str(uuid.uuid4()),
        "task_id": task_id,
        "status": "RUNNING",
        "message": "Task execution started (mock)"
    }


# ========== BULK UPLOAD ENDPOINT ==========
@router.post("/upload/bulk")
async def upload_bulk_tasks(
    file: UploadFile = File(...),
    db: Session = Depends(get_db)
):
    """Upload JSON file containing multiple tasks"""
    try:
        content = await file.read()
        data = json.loads(content)
        
        # Check if it's bulk format (has "tasks" array)
        tasks_data = data.get("tasks", [])
        
        if not tasks_data:
            # Single task format
            tasks_data = [data]
        
        logger.info(f"Uploading {len(tasks_data)} tasks")
        
        results = []
        success_count = 0
        failed_count = 0
        
        for task_data in tasks_data:
            try:
                task = create_single_task(task_data, db)
                db.flush()
                results.append({
                    "task_key": task.task_key,
                    "task_name": task.name,
                    "status": "success",
                    "task_id": str(task.id)
                })
                success_count += 1
                logger.info(f"✅ Created task: {task.task_key}")
            except Exception as e:
                results.append({
                    "task_name": task_data.get("task_name", "Unknown"),
                    "status": "failed",
                    "error": str(e)
                })
                failed_count += 1
                logger.error(f"❌ Failed to create task: {e}")
        
        # Commit all successful tasks
        db.commit()
        
        return {
            "success": True,
            "message": f"Processed {len(tasks_data)} tasks: {success_count} success, {failed_count} failed",
            "data": {
                "total": len(tasks_data),
                "success": success_count,
                "failed": failed_count,
                "results": results
            }
        }
        
    except json.JSONDecodeError as e:
        logger.error(f"Invalid JSON: {e}")
        raise HTTPException(status_code=400, detail=f"Invalid JSON format: {str(e)}")
    except Exception as e:
        logger.error(f"Upload failed: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail=f"Failed to process upload: {str(e)}")